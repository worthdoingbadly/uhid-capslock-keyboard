#include <fcntl.h>
#include <linux/uhid.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const uint8_t kDescriptor[] = {
#include "descriptor.h"
};

int main() {
  int fd = open("/dev/uhid", O_RDWR);
  {
    struct uhid_event event = {
        .type = UHID_CREATE2,
        .u.create2 =
            {
                .name = "uhid thing",
                .phys = {0},
                .uniq = {0},
                .rd_size = sizeof(kDescriptor),
                .bus = BUS_USB,
                .vendor = 0x045e,
                .product = 0x076c,
                .version = 0x1234,
                .country = 0,
            },
    };
    memcpy(event.u.create2.rd_data, kDescriptor, sizeof(kDescriptor));
    write(fd, &event, sizeof(event));
  }
  for (;;) {
    struct uhid_event in_event = {};
    int read_count = read(fd, &in_event, sizeof(in_event));
    if (read_count < 0) {
      fprintf(stderr, "nope\n");
      break;
    }
    switch (in_event.type) {
      case UHID_START:
      case UHID_OPEN:
        break;
      case UHID_OUTPUT:
        fprintf(stderr, "uhid output! size=%d rtype=%d output=%x %x %x\n",
                in_event.u.output.size, in_event.u.output.rtype,
                in_event.u.output.data[0], in_event.u.output.data[1],
                in_event.u.output.data[2]);
        break;
      default:
        printf("unknown event %d\n", in_event.type);
    }
  }
}
