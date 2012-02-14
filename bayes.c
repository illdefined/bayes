#include <endian.h>
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

bool bayes_init(struct bayes *restrict bayes,
	char const *restrict dbpath, bool wrt) {
	bayes->err = nil;
	return tchdbopen(&bayes->hdb, dbpath,
		wrt ? HDBOWRITER | HDBOCREAT : HDBOREADER);
}

bool bayes_fini(struct bayes *restrict bayes) {
	return tchdbclose(&bayes->hdb);
}

static TCMAP *tokenise(struct bayes *restrict bayes,
	char const *restrict mesg, ssize_t len) {
	TCMAP *map = tcmapnew();
	if (unlikely(!map))
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
			bayes->err
				= "Incomplete multi byte character at end of input";
			goto leave_map;
		case (size_t) -1:
			bayes->err
				= "Invalid multi byte character in input";
			goto leave_map;
		case 0:
			width = 1;
			break;
		}

		if (iswcntrl(wide)
			|| iswpunct(wide)
			|| iswspace(wide)) {
			if (head < tail) {
				if (unlikely(!tcmapputkeep(map,
					mesg + head, tail - head, nil, 0))) {
					bayes->err
						= "Failed to add token to set";
					goto leave_map;
				}
			}

			head = tail + width;
		}

		tail += width;
	}

leave_map:
	tcmapdel(map);
	map = nil;
leave:
	return map;
}

bool bayes_feed(struct bayes *restrict bayes,
	char const *restrict mesg, ssize_t len, bool match) {
	bool ret = false;

	TCMAP *map = tokenise(bayes, mesg, len);
	if (unlikely(!map))
		goto leave;

	/* Begin transaction */
	if (unlikely(!tchdbtranbegin(&bayes->hdb))) {
		bayes->err
			= "Failed to begin database transaction";
		goto leave_map;
	}

	tcmapiterinit(map);

	int toklen;
	void const *token;

	/* Insert tokens into the database */
	while (token = tcmapiternext(map, &toklen)) {
		struct item item;

		int ret = tchdbget3(&bayes->hdb, token, toklen, &item, sizeof item);
		if (ret == sizeof item) {
			if (match)
				item.match = htobe64(be64toh(item.match) + UINT64_C(1));
			item.total = htobe64(be64toh(item.match) + UINT64_C(1));
		}
		else {
			item.match = match ? UINT64_C(1) : UINT64_C(0);
			item.total = UINT64_C(1);
		}

		if (unlikely(!tchdbput(&bayes->hdb, token, toklen, &item, sizeof item))) {
			bayes->err
				= "Failed to insert item into database";
			goto leave_tran;
		}
	}

	/* Commit transaction */
	if (unlikely(!tchdbtrancommit(&bayes->hdb))) {
		bayes->err
			= "Failed to commit database transaction";
		goto leave_map;
	}

	goto leave_map;

leave_tran:
	tchdbtranabort(&bayes->hdb);
leave_map:
	tcmapdel(map);
leave:
	return ret;
}

double bayes_prob(struct bayes *restrict bayes,
	char const *restrict mesg, ssize_t len) {
	double ret = HUGE_VAL;

	TCMAP *map = tokenise(bayes, mesg, len);
	if (unlikely(!map))
		goto leave;

	/* TODO: Calculate probability */

leave_map:
	tcmapdel(map);
leave:
	return ret;
}
