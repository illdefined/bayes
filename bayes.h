#pragma once
#ifndef __bayes_h__
#define __bayes_h__

#include <defy/bool>
#include <defy/restrict>

#include <sys/types.h>

#include <tchdb.h>

struct bayes {
	TCHDB hdb;
	char const *err;
};

bool bayes_init(struct bayes *restrict, char const *restrict, bool);
bool bayes_fini(struct bayes *restrict);
bool bayes_feed(struct bayes *restrict, char const *restrict, ssize_t, bool);
double bayes_prob(struct bayes *restrict, char const *restrict, ssize_t);

#endif
