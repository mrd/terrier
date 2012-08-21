/*
 * List macros
 *
 * -------------------------------------------------------------------
 *
 * Copyright (C) 2011 Matthew Danish.
 *
 * All rights reserved. Please see enclosed LICENSE file for details.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the author nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* These "templates" generate functions and types according to the
 * 'name' parameter given. */

/* name_insert: inserts into the list at the current spot */

/* name_append: appends to the list--if linear--or inserts into the
 * list after the current spot--if circular. */

/* name_delete: deletes the current element and moves the pointer to
 * the next. */

#ifndef _LIST_H_
#define _LIST_H_

#ifndef UNUSED
#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif
#endif

/* Doubly-linked list "template" */
#define DLIST_TYPE(name,ty)                                             \
  struct _##name##_list {                                               \
    ty elt;                                                             \
    struct _##name##_list *next, *prev;                                 \
  };                                                                    \
  typedef struct _##name##_list name##_list_t;                          \
  static void UNUSED name##_insert(name##_list_t **l, name##_list_t *n) \
  {                                                                     \
    n->next = *l;                                                       \
    if (*l != NULL && (*l)->prev != NULL) {                             \
      n->prev = (*l)->prev;                                             \
      n->prev->next = n;                                                \
    } else {                                                            \
      n->prev = NULL;                                                   \
    }                                                                   \
    if (n->next != NULL) {                                              \
      n->next->prev = n;                                                \
    }                                                                   \
    *l = n;                                                             \
  }                                                                     \
                                                                        \
  static void UNUSED name##_append(name##_list_t **l, name##_list_t *n) \
  {                                                                     \
    name##_list_t *t;                                                   \
    if (*l == NULL) {                                                   \
      n->next = n->prev = NULL;                                         \
      *l = n;                                                           \
      return;                                                           \
    }                                                                   \
    n->next = NULL;                                                     \
    for (t = *l; t->next; t = t->next);                                 \
    t->next = n;                                                        \
    n->prev = t;                                                        \
  }                                                                     \
                                                                        \
  static UNUSED name##_list_t *name##_delete(name##_list_t **l)         \
  {                                                                     \
    name##_list_t *n;                                                   \
    if (*l != NULL) {                                                   \
      n = *l;                                                           \
      if (n->next == NULL)                                              \
        *l = n->prev;                                                   \
      else                                                              \
        *l = n->next;                                                   \
      if (n->next != NULL)                                              \
        n->next->prev = n->prev;                                        \
      if (n->prev != NULL)                                              \
        n->prev->next = n->next;                                        \
      return n;                                                         \
    }                                                                   \
    return NULL;                                                        \
  }

#endif

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
