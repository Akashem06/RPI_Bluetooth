#include "hardware_bl.h"

#include "irq.h"
#include "log.h"
#include "timer.h"
#include "uart.h"

void HCI_handle_hw_rx(uint8_t byte);
uint8_t HCI_buffer_space(void);
void hw_delay_ms(uint32_t ms);

/* BEGIN USER DEFINED VARIABLES */

UartSettings bt_settings = { .uart = UART0, .cts = 30, .rts = 31, .tx = 32, .rx = 33, .bluetooth = true };

/* END USER DEFIEND VARIABLES */

void handle_uart0_irq(void) {
  /* If RX interrupt. */
  if (bt_settings.uart->mis & ((1 << 4) | (1 << 6))) {
    bt_settings.uart->icr = (1 << 4) | (1 << 6);

    /* If FIFO full, Assert RTS*/
    if (bt_settings.uart->fr & (1 << 6)) {
      bt_settings.uart->cr &= ~(1 << 11);
    }

    /* Read up to 32 bytes maximum (typical FIFO size) to prevent infinite loop */
    uint32_t read_count = 0;
    while (!(bt_settings.uart->fr & (1 << 4)) && read_count < 32) {
      uint8_t byte = bt_settings.uart->dr & 0xFF;
      HCI_handle_hw_rx(byte);
      read_count++;
    }

    if (HCI_buffer_space()) {
      bt_settings.uart->cr |= (1 << 11);
    }
  }

  /* If TX interrupt. */
  if (bt_settings.uart->mis & (1 << 5)) {
    bt_settings.uart->icr |= (1 << 5);
  }

  /* If Overrun interrupt. */
  if (bt_settings.uart->mis & (1 << 10)) {
    bt_settings.uart->icr |= (1 << 10);
  }
}

void hw_init(void) {
  /* BEGIN USER DEFINED CODE */
  uart_init(&bt_settings);
  irq_init_vectors();
  enable_interrupt_controller();
  irq_enable();
  /*  END USER DEFINED CODE  */
}

void hw_transmit_byte(uint8_t byte) {
  /* BEGIN USER DEFINED CODE */
  bt_settings.uart->cr |= (1 << 11);
  uart_transmit(byte);
  /*  END USER DEFINED CODE  */
}

void hw_transmit_buffer(uint8_t *buffer, uint16_t buffer_length) {
  /* BEGIN USER DEFINED CODE */
  bt_settings.uart->cr |= (1 << 11);
  for (uint16_t i = 0; i < buffer_length; i++) {
    /* Wait for CTS and TX FIFO space */
    while (!(bt_settings.uart->fr & (1 << 0)) || (bt_settings.uart->fr & (1 << 5))) {
      hw_delay_ms(1);
    }

    bt_settings.uart->dr = buffer[i];
  }
  /*  END USER DEFINED CODE  */
}

uint8_t hw_receive_byte(void) {
  /* BEGIN USER DEFINED CODE */
  while (bt_settings.uart->fr & (1 << 4));
  return bt_settings.uart->dr & 0xFF;
  /*  END USER DEFINED CODE  */
}

void hw_delay_ms(uint32_t ms) {
  /* BEGIN USER DEFINED CODE */
  timer_sleep(ms);
  /*  END USER DEFINED CODE  */
}

uint64_t hw_get_time_ms(void) {
  /* BEGIN USER DEFINED CODE */
  return timer_get_ticks();
  /*  END USER DEFINED CODE  */
}
