#include <math.h>
#include <stdint.h>

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
	bayes->err = nil;
	return tchdbclose(&bayes->hdb);
}

bool bayes_feed(struct bayes *restrict bayes,
	char const *restrict mesg, ssize_t len, bool match) {
	bool ret = false;

	TCMDB *mdb = tcmdbnew();
	if (unlikely(!mdb))
		goto leave;

	/* TODO: Tokenise message */

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
