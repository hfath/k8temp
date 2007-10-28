/*
 * k8temp functions for scanning PCI devices and reading/writing
 * to registers on them, via NetBSD-style libpci and /dev/pci0
 *
 * Untested, unfinished. Beware of dog.
 */

#include "k8temp_libpci.h"

int fd;

// perform any setup, exit on failure
void k8_pci_init()
{
	fd = open(_PATH_DEVPCI, O_RDWR, 0);

	if (fd < 0)
		err(EXIT_FAILURE, "open(\"%s\")", _PATH_DEVPCI);
}

void k8_pci_close()
{
	if (fd) close(fd);
}

int k8_pci_vendor_device_list(int vendor_id, int device_id, k8_pcidev_t devs[], int maxdev)
{
	int matches = -1;
	u_int8_t dev,func;
	int pcireg;
	k8_pcidev_t sel;
	bzero(&sel, sizeof(k8_pcidev_t));
	sel.bus = 0;
	for (dev=0; dev < 32; dev++)
	{
		sel.dev = dev;
		for (func=0; func < 8; func++)
		{
			sel.func = func;
			if (k8_pci_read_word(sel, 0, &pcireg))
			{
				if ((pcireg & 0xffff) == vendor_id &&
				   ((pcireg >> 16) & 0xffff) == device_id &&
				   matches < maxdev)
				{
					memcpy(&devs[++matches], &sel, sizeof(k8_pcidev_t));
				}
			}
		}
	}
	return(matches);
}

// k8_pci_read_*() fill data with the value at offset,
// on failure, print diagnostic and return 0

int k8_pci_read(k8_pcidev_t dev, int offset, int *data, int width)
{
	if (pcibus_conf_read(fd, dev.bus, dev.dev, dev.func, offset, &data) < 0)
	{
		warn("Register read %x, %d bytes failed", offset, width);
		return(0);
	}

	switch (width)
	{
	case 1:
		*buf = *buf & 0xf;
		break;
	case 4:
		break;
	default:
		warnx("Width %d register reads unsupported", width);
	}
	return(width);
}

int k8_pci_read_byte(k8_pcidev_t dev, int offset, int *data)
{
	return(k8_pci_read(dev, offset, data, 1));
}

int k8_pci_read_word(k8_pcidev_t dev, int offset, int *data)
{
	return(k8_pci_read(dev, offset, data, 4));
}

// same, but for writes.
int k8_pci_write(k8_pcidev_t dev, int offset, int data, int width)
{
	/* XXX: we don't bother with read-modify-write, as our specific
	 * use is known-safe without it.  Beware if you want to use this
	 * for anything else, though.
	 */
	if (pcibus_conf_write(fd, dev.bus, dev.dev, dev.func, offset, data) < 0)
	{
		warn("Register write %x, %d bytes failed", offset, width);
		return(0);
	}

	return(width);
}

int k8_pci_write_byte(k8_pcidev_t dev, int offset, int data)
{
//	return(1);
	return(k8_pci_write(dev, offset, data, 1));
}

int k8_pci_write_word(k8_pcidev_t dev, int offset, int data)
{
//	return(4);
	return(k8_pci_write(dev, offset, data, 4));
}
