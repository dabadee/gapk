/* Processing Queue Management */

/*
 * This file is part of gapk (GSM Audio Pocket Knife).
 *
 * gapk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gapk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gapk.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include <gapk/procqueue.h>


#define MAX_PQ_ITEMS	8

struct pq {
	int		n_items;
	struct pq_item* items[MAX_PQ_ITEMS];
	void *		buffers[MAX_PQ_ITEMS+1];
};


struct pq *
pq_create(void)
{
	return (struct pq *) calloc(1, sizeof(struct pq));
}

void
pq_destroy(struct pq *pq)
{
	int i;

	if (!pq)
		return;

	for (i=0; i<pq->n_items; i++) {
		if (!pq->items[i])
			continue;
		if (pq->items[i]->exit)
			pq->items[i]->exit(pq->items[i]->state);
		free(pq->items[i]);
	}

	for (i=0; i<pq->n_items-1; i++)
		free(pq->buffers[i]); /* free is NULL safe */

	free(pq);
}

struct pq_item *
pq_add_item(struct pq *pq)
{
	struct pq_item *item;

	if (pq->n_items == MAX_PQ_ITEMS)
		return NULL;

	item = calloc(1, sizeof(struct pq_item));
	if (!item)
		return NULL;

	pq->items[pq->n_items++] = item;

	return item;
}

int
pq_prepare(struct pq *pq)
{
	int i;
	unsigned int len_prev;

	len_prev = 0;

	for (i=0; i<pq->n_items; i++) {
		struct pq_item *item = pq->items[i];

		if (item->len_in != len_prev)
			return -EINVAL;

		if (i < (pq->n_items-1)) {
			pq->buffers[i] = malloc(item->len_out);
			if (!pq->buffers[i])
				return -ENOMEM;
		} else{
			if (item->len_out)
				return -EINVAL;
		}

		len_prev = item->len_out;
	}

	return 0;
}

int
pq_execute(struct pq *pq)
{
	int i;
	void *buf_prev, *buf;

	buf_prev = NULL;

	for (i=0; i<pq->n_items; i++) {
		int rv;
		struct pq_item *item = pq->items[i];

		buf = i < (pq->n_items-1) ? pq->buffers[i] : NULL;

		rv = item->proc(item->state, buf, buf_prev);
		if (rv)
			return rv;

		buf_prev = buf;
	}

	return 0;
}