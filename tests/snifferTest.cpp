
#include "stdafx.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../libs/libutils/logging.h"
#include "../libs/libutils/Exception.h"
#include "../libs/librf/spidev_lib++.h"
#include "../libs/librf/RFM69OOKregisters.h"
#include "../libs/librf/RFM69OOK.h"
#include "../libs/librf/RFParser.h"
#include "../libs/librf/RFProtocolLivolo.h"
#include "../libs/librf/RFProtocolX10.h"
#include "../libs/librf/RFProtocolRST.h"
#include "../libs/librf/RFProtocolRaex.h"

typedef uint32_t __u32;
#define LIRC_GET_REC_MODE              _IOR('i', 0x00000002, __u32)
#define lirc_t base_type
#define PULSE_BIT       0x01000000
#define PULSE_MASK      0x00FFFFFF


int waitfordata(int fd, unsigned long maxusec)
{
#ifndef WIN32
	fd_set fds;
	int ret;
	struct timeval tv;

	while (1) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		do { 
			do {
				if (maxusec > 0) {
					tv.tv_sec = maxusec / 1000000;
					tv.tv_usec = maxusec % 1000000;
					ret = select(fd + 1, &fds, NULL, NULL, &tv);
					if (ret == 0)
						return (0);
				} else {
					ret = select(fd + 1, &fds, NULL, NULL, NULL);
				}
			}
			while (ret == -1 && errno == EINTR);
			if (ret == -1) {
				CLog::GetLog("Main")->Printf(0, "RF select() failed\n");
				continue;
			}
		}
		while (ret == -1);

		if (FD_ISSET(fd, &fds)) {
			/* we will read later */
			return (1);
		}
	}
#endif
	return false;
}

void SnifferTest()
{
    printf("Rfm69Test\n");
	string m_lircDevice="/dev/lirc0";
	CLog *m_Log = CLog::GetLog("Main");
	int fd = open(m_lircDevice.c_str(), O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "error opening %s\n", m_lircDevice.c_str());
		exit(EXIT_FAILURE);
	};

	struct stat s;
	__u32 mode = 2;

	if ((fstat(fd, &s) != -1) && (S_ISFIFO(s.st_mode))) {
		/* can't do ioctls on a pipe */
	}
	else if ((fstat(fd, &s) != -1) && (!S_ISCHR(s.st_mode))) {
		close(fd);
		throw CHaException(CHaException::ErrBadParam,"%s is not a character device\n", m_lircDevice.c_str());
	}
	else if (ioctl(fd, LIRC_GET_REC_MODE, &mode) == -1) {
		throw CHaException(CHaException::ErrBadParam,"This program is only intended for receivers supporting the pulse/space layer.");
	}

	CRFParser m_parser(m_Log);
	m_parser.AddProtocol(new CRFProtocolRST());
	m_parser.AddProtocol(new CRFProtocolLivolo());
	m_parser.AddProtocol(new CRFProtocolX10());
	m_parser.AddProtocol(new CRFProtocolRaex());


	const size_t BUFFER_SIZE = 1024 * 1024;
	lirc_t *data = new lirc_t[BUFFER_SIZE];
	lirc_t *data_ptr = data;
	time_t lastReport = 0, packetStart = time(NULL);
	bool m_bExecute=true;
	while (m_bExecute) {
		int result;
		usleep(10);

		size_t count = sizeof(lirc_t)*BUFFER_SIZE - (data_ptr - data)*sizeof(lirc_t);

		if (count == 0)
		{
			m_Log->Printf(0, "RF buffer full");
			data_ptr = data;
			continue;
		}

		if (lastReport != time(NULL) && data_ptr != data)
		{
			//printf ("read for %ld bytes from %ld\n", count, data_ptr-data);
			m_Log->Printf(100,"RF got data %ld bytes", data_ptr - data);
			lastReport = time(NULL);
		}

		if (data_ptr == data)
			packetStart = time(NULL);
		else if (!waitfordata(fd, 1000000) || data_ptr - data>10000)
		{
			string parsedResult = m_parser.Parse(data, data_ptr - data);
			if (parsedResult.length())
			{
				m_Log->Printf(3, "RF Recieved: %s\n", parsedResult.c_str());
				m_bExecute = false;
			}
			else
			{
				m_Log->Printf(10, "Recieved %ld signals. Not decoded\n", data_ptr - data);
			}
			data_ptr = data;
			packetStart = time(NULL);
		}

		result = read(fd, (void *)data_ptr, count);
		if (result == 0) {
			fprintf(stderr, "read() failed\n");
			break;
		}

		data_ptr += result / sizeof(lirc_t);
	};

	delete []data;
	close(fd);
}
