#include <fcntl.h>
#include <linux/uhid.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const uint8_t kDescriptor[] = {
#include "descriptor.h"
};

int main1(int argc, char** argv) {
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
                .vendor = 0x0596,
                .product = 0x0500,  // USB_DEVICE_ID_3M1968
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
        fprintf(stderr, "start\n");
	break;
      case UHID_OPEN:
        fprintf(stderr, "open\n");
	close(fd);
	return 0;
        break;
      case UHID_STOP:
        fprintf(stderr, "stop\n");
        break;
      case UHID_CLOSE:
        fprintf(stderr, "close\n");
        return 1;
      case UHID_OUTPUT:
        fprintf(stderr, "uhid output! size=%d rtype=%d output=%x %x %x\n",
                in_event.u.output.size, in_event.u.output.rtype,
                in_event.u.output.data[0], in_event.u.output.data[1],
                in_event.u.output.data[2]);
        break;
      case UHID_GET_REPORT: {
        fprintf(stderr, "get report id=%x rnum=%x rtype=%x\n",
                in_event.u.get_report.id, in_event.u.get_report.rnum,
                in_event.u.get_report.rtype);
        struct uhid_event event = {.type = UHID_GET_REPORT_REPLY,
                                   .u.get_report_reply = {
                                       .id = in_event.u.get_report.id,
                                       .err = 0,
                                       .size = argc == 1? 0: 1,
                                   }};
        uint8_t outdata = argc == 2 ? atoi(argv[1]) : 0;
        fprintf(stderr, "returning data=%x\n", outdata);
        event.u.get_report_reply.data[0] = outdata;
        write(fd, &event, sizeof(event));
        break;
      }
      case UHID_SET_REPORT:
        fprintf(stderr, "set report\n");
        break;
      default:
        printf("unknown event %d\n", in_event.type);
    }
  }
}

int main(int argc, char** argv) {
	main1(argc, argv);
	main1(1, argv);
}
