#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include <defy/bool>
#include <defy/expect>
#include <defy/nil>
#include <defy/restrict>

#include <tchdb.h>

#include "bayes.h"

struct item {
	uint64_t match;
	uint64_t total;
};

bool bayes_init(struct bayes *restrict bayes, char const *restrict dbpath, bool wrt) {
	bayes->err = nil;
	return tchdbopen(&bayes->hdb, dbpath, wrt ? HDBOWRITER | HDBOCREAT : HDBOREADER);
}

bool bayes_fini(struct bayes *restrict bayes) {
	return tchdbclose(&bayes->hdb);
}

bool bayes_feed(struct bayes *restrict bayes,
	char const *restrict mesg, ssize_t len, bool match) {
	bool ret = false;

	TCMDB *mdb = tcmdbnew();
	if (unlikely(!mdb))
		goto leave;

	size_t head = 0;
	size_t tail = 0;

	mbstate_t mbs;
	memset(&mbs, 0, sizeof mbs);

	while (tail < len) {
		wchar_t wide;
		size_t width
			= mbrtowc(&wide, mesg + tail, len - tail, &mbs);
		switch (width) {
		case (size_t) -2:
			bayes->err = "Incomplete multi byte character at end of input";
			goto leave_mdb;
		case (size_t) -1:
			bayes->err = "Invalid multi byte character in input";
			goto leave_mdb;
		case 0:
			width = 1;
			break;
		}

		if (iswcntrl(wide)
			|| iswspace(wide)) {
			if (head < tail) {
				/* TODO: Process token */
			}

			head = tail + width;
		}

		tail += width;
	}

leave_mdb:
	tcmdbdel(mdb);
leave:
	return ret;
}

double bayes_prob(struct bayes *restrict bayes,
	char const *restrict mesg, ssize_t len) {
	double ret = HUGE_VAL;
	return ret;
}
