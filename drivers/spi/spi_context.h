/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Private API for SPI drivers
 */

#ifndef __SPI_DRIVER_COMMON_H__
#define __SPI_DRIVER_COMMON_H__

#include <gpio.h>
#include <spi.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spi_context {
	struct spi_config *config;

	const struct spi_buf **current_tx;
	struct spi_buf **current_rx;

	void *tx_buf;
	u32_t tx_len;
	void *rx_buf;
	u32_t rx_len;
};

static inline bool spi_context_configured(struct spi_context *ctx,
					  struct spi_config *config)
{
	return !!(ctx->config == config);
}

static inline void spi_context_cs_configure(struct spi_context *ctx)
{
	if (ctx->config->cs) {
		gpio_pin_configure(ctx->config->cs->gpio_dev,
				   ctx->config->cs->gpio_pin, GPIO_DIR_OUT);
		gpio_pin_write(ctx->config->cs->gpio_dev,
			       ctx->config->cs->gpio_pin, 1);
	}
}

static inline void spi_context_cs_control(struct spi_context *ctx, bool on)
{
	if (ctx->config->cs) {
		if (on) {
			gpio_pin_write(ctx->config->cs->gpio_dev,
				       ctx->config->cs->gpio_pin, 0);
			k_busy_wait(ctx->config->cs->delay);
		} else {
			k_busy_wait(ctx->config->cs->delay);
			gpio_pin_write(ctx->config->cs->gpio_dev,
				       ctx->config->cs->gpio_pin, 1);
		}
	}
}

static inline void spi_context_buffers_setup(struct spi_context *ctx,
					     const struct spi_buf **tx_bufs,
					     struct spi_buf **rx_bufs,
					     uint8_t dfs)
{
	SYS_LOG_DBG("tx_bufs %p (%p) - rx_bufs %p (%p) - %u",
		    tx_bufs, tx_bufs ? *tx_bufs : NULL,
		    rx_bufs, rx_bufs ? *rx_bufs : NULL, dfs);

	ctx->current_tx = tx_bufs;
	ctx->current_rx = rx_bufs;

	if (*tx_bufs) {
		ctx->tx_buf = (*tx_bufs)->buf;
		ctx->tx_len = (*tx_bufs)->len/dfs;
	} else {
		ctx->tx_buf = NULL;
		ctx->tx_len = 0;
	}

	if (*rx_bufs) {
		ctx->rx_buf = (*rx_bufs)->buf;
		ctx->rx_len = (*rx_bufs)->len/dfs;
	} else {
		ctx->rx_buf = NULL;
		ctx->rx_len = 0;
	}

	SYS_LOG_DBG("current_tx %p, current_rx %p,"
		    " tx buf/len %p/%u, rx buf/len %p/%u",
		    ctx->current_tx, ctx->current_rx,
		    ctx->tx_buf, ctx->tx_len, ctx->rx_buf, ctx->rx_len);
}

static ALWAYS_INLINE
void spi_context_update_tx(struct spi_context *ctx, uint8_t dfs)
{
	if (!ctx->tx_len) {
		return;
	}

	ctx->tx_len--;
	if (!ctx->tx_len) {
		ctx->current_tx++;
		if (*ctx->current_tx) {
			ctx->tx_buf = (*ctx->current_tx)->buf;
			ctx->tx_len = (*ctx->current_tx)->len/dfs;
		} else {
			ctx->tx_buf = NULL;
		}
	} else if (ctx->tx_buf) {
		ctx->tx_buf += dfs;
	}

	SYS_LOG_DBG("tx buf/len %p/%u", ctx->tx_buf, ctx->tx_len);
}

static ALWAYS_INLINE
bool spi_context_tx_on(struct spi_context *ctx)
{
	return !!(ctx->tx_buf || ctx->tx_len);
}

static ALWAYS_INLINE
void spi_context_update_rx(struct spi_context *ctx, uint8_t dfs)
{
	if (!ctx->rx_len) {
		return;
	}

	ctx->rx_len--;
	if (!ctx->rx_len) {
		ctx->current_rx++;
		if (*ctx->current_rx) {
			ctx->rx_buf = (*ctx->current_rx)->buf;
			ctx->rx_len = (*ctx->current_rx)->len/dfs;
		} else {
			ctx->rx_buf = NULL;
		}
	} else if (ctx->rx_buf) {
		ctx->rx_buf += dfs;
	}

	SYS_LOG_DBG("rx buf/len %p/%u", ctx->rx_buf, ctx->rx_len);
}

static ALWAYS_INLINE
bool spi_context_rx_on(struct spi_context *ctx)
{
	return !!(ctx->rx_buf || ctx->rx_len);
}

#ifdef __cplusplus
}
#endif

#endif /* __SPI_DRIVER_COMMON_H__ */