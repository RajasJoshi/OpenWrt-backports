// SPDX-License-Identifier: GPL-2.0-only
/*
 * SPI interface.
 *
 * Copyright (c) 2017-2020, Silicon Laboratories, Inc.
 * Copyright (c) 2011, Sagrad Inc.
 * Copyright (c) 2010, ST-Ericsson
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/pm.h>

#include "bus.h"
#include "wfx.h"
#include "hwio.h"
#include "main.h"
#include "bh.h"

#define SET_WRITE 0x7FFF        /* usage: and operation */
#define SET_READ 0x8000         /* usage: or operation */

static const struct wfx_platform_data pdata_wf200 = {
	.file_fw = "wfx/wfm_wf200",
	.file_pds = "wfx/wf200.pds",
	.use_rising_clk = true,
};

static const struct wfx_platform_data pdata_brd4001a = {
	.file_fw = "wfx/wfm_wf200",
	.file_pds = "wfx/brd4001a.pds",
	.use_rising_clk = true,
};

static const struct wfx_platform_data pdata_brd8022a = {
	.file_fw = "wfx/wfm_wf200",
	.file_pds = "wfx/brd8022a.pds",
	.use_rising_clk = true,
};

static const struct wfx_platform_data pdata_brd8023a = {
	.file_fw = "wfx/wfm_wf200",
	.file_pds = "wfx/brd8023a.pds",
	.use_rising_clk = true,
};

struct wfx_spi_priv {
	struct spi_device *func;
	struct wfx_dev *core;
	struct gpio_desc *gpio_reset;
	bool need_swab;
};

/* The chip reads 16bits of data at time and place them directly into (little endian) CPU register.
 * So, the chip expects bytes order to be "B1 B0 B3 B2" (while LE is "B0 B1 B2 B3" and BE is
 * "B3 B2 B1 B0")
 *
 * A little endian host with bits_per_word == 16 should do the right job natively. The code below to
 * support big endian host and commonly used SPI 8bits.
 */
static int wfx_spi_copy_from_io(void *priv, unsigned int addr, void *dst, size_t count)
{
	struct wfx_spi_priv *bus = priv;
	u16 regaddr = (addr << 12) | (count / 2) | SET_READ;
	struct spi_message m;
	struct spi_transfer t_addr = {
		.tx_buf = &regaddr,
		.len = sizeof(regaddr),
	};
	struct spi_transfer t_msg = {
		.rx_buf = dst,
		.len = count,
	};
	u16 *dst16 = dst;
	int ret, i;

	WARN(count % 2, "buffer size must be a multiple of 2");

	cpu_to_le16s(&regaddr);
	if (bus->need_swab)
		swab16s(&regaddr);

	spi_message_init(&m);
	spi_message_add_tail(&t_addr, &m);
	spi_message_add_tail(&t_msg, &m);
	ret = spi_sync(bus->func, &m);

	if (bus->need_swab && addr == WFX_REG_CONFIG)
		for (i = 0; i < count / 2; i++)
			swab16s(&dst16[i]);
	return ret;
}

static int wfx_spi_copy_to_io(void *priv, unsigned int addr, const void *src, size_t count)
{
	struct wfx_spi_priv *bus = priv;
	u16 regaddr = (addr << 12) | (count / 2);
	/* FIXME: use a bounce buffer */
	u16 *src16 = (void *)src;
	int ret, i;
	struct spi_message m;
	struct spi_transfer t_addr = {
		.tx_buf = &regaddr,
		.len = sizeof(regaddr),
	};
	struct spi_transfer t_msg = {
		.tx_buf = src,
		.len = count,
	};

	WARN(count % 2, "buffer size must be a multiple of 2");
	WARN(regaddr & SET_READ, "bad addr or size overflow");

	cpu_to_le16s(&regaddr);

	/* Register address and CONFIG content always use 16bit big endian
	 * ("BADC" order)
	 */
	if (bus->need_swab)
		swab16s(&regaddr);
	if (bus->need_swab && addr == WFX_REG_CONFIG)
		for (i = 0; i < count / 2; i++)
			swab16s(&src16[i]);

	spi_message_init(&m);
	spi_message_add_tail(&t_addr, &m);
	spi_message_add_tail(&t_msg, &m);
	ret = spi_sync(bus->func, &m);

	if (bus->need_swab && addr == WFX_REG_CONFIG)
		for (i = 0; i < count / 2; i++)
			swab16s(&src16[i]);
	return ret;
}

static void wfx_spi_lock(void *priv)
{
}

static void wfx_spi_unlock(void *priv)
{
}

static irqreturn_t wfx_spi_irq_handler(int irq, void *priv)
{
	struct wfx_spi_priv *bus = priv;

	wfx_bh_request_rx(bus->core);
	return IRQ_HANDLED;
}

static int wfx_spi_irq_subscribe(void *priv)
{
	struct wfx_spi_priv *bus = priv;
	u32 flags;

	flags = irq_get_trigger_type(bus->func->irq);
	if (!flags)
		flags = IRQF_TRIGGER_HIGH;
	flags |= IRQF_ONESHOT;
	return devm_request_threaded_irq(&bus->func->dev, bus->func->irq, NULL,
					 wfx_spi_irq_handler, flags, "wfx", bus);
}

static int wfx_spi_irq_unsubscribe(void *priv)
{
	struct wfx_spi_priv *bus = priv;

	devm_free_irq(&bus->func->dev, bus->func->irq, bus);
	return 0;
}

static size_t wfx_spi_align_size(void *priv, size_t size)
{
	/* Most of SPI controllers avoid DMA if buffer size is not 32bit aligned */
	return ALIGN(size, 4);
}

static void wfx_spi_set_wakeup(void *priv, bool enabled)
{
	struct wfx_spi_priv *bus = priv;

	device_set_wakeup_enable(&bus->func->dev, enabled);
}

static const struct wfx_hwbus_ops wfx_spi_hwbus_ops = {
	.copy_from_io    = wfx_spi_copy_from_io,
	.copy_to_io      = wfx_spi_copy_to_io,
	.irq_subscribe   = wfx_spi_irq_subscribe,
	.irq_unsubscribe = wfx_spi_irq_unsubscribe,
	.lock            = wfx_spi_lock,
	.unlock          = wfx_spi_unlock,
	.align_size      = wfx_spi_align_size,
	.set_wakeup      = wfx_spi_set_wakeup,
};

static int wfx_spi_suspend(struct device *dev)
{
	struct spi_device *func = to_spi_device(dev);
	struct wfx_spi_priv *bus = spi_get_drvdata(func);

	if (!device_may_wakeup(dev))
		return 0;
	flush_work(&bus->core->hif.bh);
	return enable_irq_wake(func->irq);
}

static int wfx_spi_resume(struct device *dev)
{
	struct spi_device *func = to_spi_device(dev);

	if (!device_may_wakeup(dev))
		return 0;
	return disable_irq_wake(func->irq);
}

static int wfx_spi_probe(struct spi_device *func)
{
	struct wfx_platform_data *pdata;
	struct wfx_spi_priv *bus;
	int ret;

	if (!func->bits_per_word)
		func->bits_per_word = 16;
	ret = spi_setup(func);
	if (ret)
		return ret;
	pdata = (struct wfx_platform_data *)spi_get_device_id(func)->driver_data;
	if (!pdata) {
		dev_err(&func->dev, "unable to retrieve driver data (please report)\n");
		return -ENODEV;
	}

	/* Trace below is also displayed by spi_setup() if compiled with DEBUG */
	dev_dbg(&func->dev, "SPI params: CS=%d, mode=%d bits/word=%d speed=%d\n",
		spi_get_chipselect(func, 0), func->mode, func->bits_per_word, func->max_speed_hz);
	if (func->bits_per_word != 16 && func->bits_per_word != 8)
		dev_warn(&func->dev, "unusual bits/word value: %d\n", func->bits_per_word);
	if (func->max_speed_hz > 50000000)
		dev_warn(&func->dev, "%dHz is a very high speed\n", func->max_speed_hz);

	bus = devm_kzalloc(&func->dev, sizeof(*bus), GFP_KERNEL);
	if (!bus)
		return -ENOMEM;
	bus->func = func;
	if (func->bits_per_word == 8 || IS_ENABLED(CONFIG_CPU_BIG_ENDIAN))
		bus->need_swab = true;
	spi_set_drvdata(func, bus);

	bus->gpio_reset = devm_gpiod_get_optional(&func->dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(bus->gpio_reset))
		return PTR_ERR(bus->gpio_reset);
	if (!bus->gpio_reset) {
		dev_warn(&func->dev, "gpio reset is not defined, trying to load firmware anyway\n");
	} else {
		gpiod_set_consumer_name(bus->gpio_reset, "wfx reset");
		gpiod_set_value_cansleep(bus->gpio_reset, 1);
		usleep_range(100, 150);
		gpiod_set_value_cansleep(bus->gpio_reset, 0);
		usleep_range(2000, 2500);
	}

	bus->core = wfx_init_common(&func->dev, pdata, &wfx_spi_hwbus_ops, bus);
	if (!bus->core)
		return -EIO;

	ret = wfx_probe(bus->core);
	if (ret)
		return ret;

	device_set_wakeup_capable(&func->dev, true);
	return 0;
}

static void wfx_spi_remove(struct spi_device *func)
{
	struct wfx_spi_priv *bus = spi_get_drvdata(func);

	wfx_release(bus->core);
}
#if LINUX_VERSION_IS_LESS(5,18,0)
static int bp_wfx_spi_remove(struct spi_device *spi){
	wfx_spi_remove(spi);

	return 0;
}
#endif

/* For dynamic driver binding, kernel does not use OF to match driver. It only
 * use modalias and modalias is a copy of 'compatible' DT node with vendor
 * stripped.
 */
static const struct spi_device_id wfx_spi_id[] = {
	{ "wf200",    (kernel_ulong_t)&pdata_wf200 },
	{ "brd4001a", (kernel_ulong_t)&pdata_brd4001a },
	{ "brd8022a", (kernel_ulong_t)&pdata_brd8022a },
	{ "brd8023a", (kernel_ulong_t)&pdata_brd8023a },
	{ },
};
MODULE_DEVICE_TABLE(spi, wfx_spi_id);

#ifdef CONFIG_OF
static const struct of_device_id wfx_spi_of_match[] = {
	{ .compatible = "silabs,wf200" },
	{ .compatible = "silabs,brd4001a" },
	{ .compatible = "silabs,brd8022a" },
	{ .compatible = "silabs,brd8023a" },
	{ },
};
MODULE_DEVICE_TABLE(of, wfx_spi_of_match);
#endif

static DEFINE_SIMPLE_DEV_PM_OPS(wfx_spi_pm_ops, wfx_spi_suspend, wfx_spi_resume);

struct spi_driver wfx_spi_driver = {
	.id_table = wfx_spi_id,
	.probe = wfx_spi_probe,
#if LINUX_VERSION_IS_GEQ(5,18,0)
	.remove = wfx_spi_remove,
#else
	.remove = bp_wfx_spi_remove,
#endif
	
	.driver = {
		.name = "wfx-spi",
		.of_match_table = of_match_ptr(wfx_spi_of_match),
		.pm = &wfx_spi_pm_ops,
	},
};
