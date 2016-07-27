/* masterPrimXface.h */

#ifndef MASTERPRIMXFACE_H
#define MASTERPRIMXFACE_H

enum {
	MASTER_DOWNLOAD_HISTOGRAM_RESULTS = 1
};

#define COPY_BUFFER 1
#define WAIT_REPLY 2

typedef struct {
	UINT32 nPoints;
	UINT32 length;
	UINT32 dataPtr;
} DownloadHistogramResultsIn;

#endif

