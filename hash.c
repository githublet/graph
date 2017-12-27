#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "hash.h"

static hash_func_data_t *
hfd_create(int num, ht_size_t ht_size, int i);

static int
hfd_destroy(hash_func_data_t *hfd);

static ht_size_t 
get_ht_index(hash_func_data_t *hfd);

static ht_size_t
is_prime(ht_size_t x);

static ht_size_t
get_next_prime(ht_size_t x);

static int 
ht_resize(ht_t *ht, ht_size_t new_bs);

static int
ht_resize_up(ht_t *ht);

static int
ht_resize_down(ht_t *ht);

static void
safe_free(void **pp);

#define sfree(p) safe_free((void**)(&(p)))

static const ht_size_t PRIME_ARRAY[]={1009,3389,7789,15919,32009,58889,89899};

ht_item_t *
ht_item_create(int k, void *v)
{
	ht_item_t *i = malloc(sizeof(ht_item_t));
	if (!i)
	{
		return NULL;
	} // end if
	
    i->k = k;
	i->v = v;
	return i;
} // end ht_item_create()

int
_ht_item_destroy(ht_item_t *i, data_dtor_func_t dtor)
{
	if (!i || !dtor)
	{
		return -1;
	} // end if
	
	dtor(i->v);
	sfree(i);
	return 0;
} // end _ht_item_destroy()

int
ht_item_destroy(ht_item_t *i)
{
	if (!i || !i->container)
	{
		return -1;
	} // end if
	
	return _ht_item_destroy(i, i->container->data_dtor);
} // end ht_item_destroy()

ht_t *
_ht_create(int bs, data_dtor_func_t dtor, prime_finder_func_t pf, hash_func_t hf)
{
	if (!dtor)
	{
		return NULL;
	} // end if
	
	ht_t *ht = malloc(sizeof(ht_t));
	if (!ht)
	{
		return NULL;
	} // end if
	
    if (bs < PRIME_ARRAY[0])
	{
        ht->base_size = PRIME_ARRAY[0];
	} // end if
	else
	{
		ht->base_size = bs;
	} // end else
	
	if (!pf)
	{
		ht->prime_finder = get_next_prime;
	} // end if
	else
	{
		ht->prime_finder = pf;
	} // end else
	
    ht->size = ht->base_size;
	
	ht->count = 0;
	
	ht->items = calloc(ht->size, sizeof(ht_item_t *));
	if (!ht->items)
	{
		return NULL;
	} // end if
	
	ht->data_dtor = dtor;
	
	if (hf)
	{
		ht->hash_func = hf;
	} // end if
	else
	{
		ht->hash_func = get_ht_index;
	} // end else
	
	return ht;
} // end _ht_create()

ht_t *
ht_create(data_dtor_func_t dtor)
{
	if (!dtor)
	{
		return NULL;
	} // end if
	
    return _ht_create(PRIME_ARRAY[0], dtor, NULL, NULL);
} // end ht_create()

int
ht_destroy(ht_t *ht)
{
	if (!ht || !ht->items)
	{
		return -1;
	} // end if
	
	int i = 0;
	ht_item_t *curr_item = NULL;
	for (; i < ht->size; i++)
	{
		curr_item = ht->items[i];
        if (curr_item != NULL)
		{
			int res = ht_item_destroy(curr_item);
			if (res == -1)
			{
				return -1;
			} // end if
            ht->items[i] = NULL;
		} // end if
	} // end for
	
	sfree(ht->items);
	sfree(ht);
	return 0;
} // end ht_destroy()

static hash_func_data_t *
hfd_create(int num, ht_size_t ht_size, int i)
{	
	hash_func_data_t *hfd = malloc(sizeof(hash_func_data_t));
	if (!hfd)
	{
		return NULL;
	} // end if
	
    hfd->num = num;
	hfd->ht_size = ht_size;
	hfd->i = i;
	return hfd;
} // end hfd_create()

static int
hfd_destroy(hash_func_data_t *hfd)
{
	if (!hfd)
	{
		return -1;
	} // end if
	
    sfree(hfd);
	return 0;
} // end hfd_destroy()

static ht_size_t 
get_ht_index(hash_func_data_t *hfd)
{
    if(!hfd)
    {
        return -1;
    }
    return (hfd->num)%(hfd->ht_size);
} // end get_ht_index()

int
ht_add(ht_t *ht, ht_item_t *item)
{
	if (!ht || !ht->items || !ht->hash_func || !item)
	{
		return -1;
	} // end if
		
	hash_func_data_t *hfd = hfd_create(item->k, ht->size, 0);
	if (!hfd)
	{
		return -1;
	} // end if
	
	ht_size_t index = ht->hash_func(hfd);
    int res = hfd_destroy(hfd);
    if (index == -1 || res == -1)
    {
        return -1;
    } // end if
	ht_item_t *curr_item = ht->items[index];

    if(curr_item != NULL || ht->count==ht->size)
    {
        res = ht_resize_up(ht);
        if(res==-1)
        {
            return -1;
        }
        hfd = hfd_create(item->k,ht->size,0);
		index = ht->hash_func(hfd);
        res = hfd_destroy(hfd);
        if (index == -1 || res == -1)
        {
            return -1;
        } // end if
    }
	item->container = ht;
	ht->items[index] = item;
	(ht->count)++;
	return 0;
} // end ht_add()

int
ht_crash(ht_t *ht,int key){
    ht_item_t *item = ht_item_create(key,NULL);
    if(!item)
    {
        printf("error get from ht_crash()\n");
        return 0;
    }
    hash_func_data_t *hfd = hfd_create(item->k,ht->size,0);
    if(!hfd){
        printf("error get from ht_crash()\n");
        return 0;
    }
    ht_size_t index = ht->hash_func(hfd);
    int res = hfd_destroy(hfd);
    if (index == -1 || res == -1)
    {
        return -1;
    } // end if
    ht_item_t *curr_item = ht->items[index];
    if(curr_item!=NULL)
        return 1;
    item->container=ht;
    ht->items[index]=item;
    (ht->count)++;
    return 0;
}

int
ht_tf(ht_t *ht, ht_item_t *item)
{
    if (!ht || !ht->items || !ht->hash_func || !item)
    {
        return -1;
    } // end if

    hash_func_data_t *hfd = hfd_create(item->k, ht->size, 0);
    if (!hfd)
    {
        return -1;
    } // end if

    ht_size_t index = ht->hash_func(hfd);
    int res = hfd_destroy(hfd);
    if (index == -1||res == -1)
    {
        return -1;
    }// end if

    ht->items[index] = item;
    (ht->count)++;
    return 0;
}

void *
ht_search(ht_t *ht, int k)
{
    if (!ht || !ht->items || !ht->hash_func)
	{
		return NULL;
	} // end if

	hash_func_data_t *hfd = hfd_create(k, ht->size, 0);
	if (!hfd)
	{
		return NULL;
	} // end if
	
	ht_size_t index = ht->hash_func(hfd);
    int res = hfd_destroy(hfd);
    if (index == -1 || res == -1)
	{	
		return NULL;
	}// end if
	
	ht_item_t *curr_item = ht->items[index];
    if (curr_item != NULL)
	{
        if (curr_item->k == k)
		{
			return curr_item->v;
		} // end if
        else
        {
            printf("error get from ht_search & hash_func\n");
        }
    }
	return NULL;
} // end ht_search()

int
ht_delete(ht_t *ht, int k)
{
    if (!ht || !ht->items || !ht->hash_func)
	{
		return -1;
	} // end if
		
	hash_func_data_t *hfd = hfd_create(k, ht->size, 0);
	if (!hfd)
	{
		return -1;
	} // end if
	
	ht_size_t index = ht->hash_func(hfd);
    int res = hfd_destroy(hfd);
    if (index == -1 || res == -1)
	{
		return -1;
	} // end if
	ht_item_t *curr_item = ht->items[index];
    if (curr_item != NULL)
	{
        if (curr_item->k == k)
		{
			int res = ht_item_destroy(curr_item);
			if (res == -1)
			{
				return -1;
			} // end if
            ht->items[index] = NULL;
			ht->count--;
			return 0;
        }
        else
        {
            printf("error get from ht_search & hash_func\n");
        }

    }
	return -1;
} // end ht_delete()

static ht_size_t
is_prime(ht_size_t x)
{
	if (x < 2)
	{
		return 0;
	} // end if
	
	if (x < 4)
	{
		return 1;
	} // end if
	
	if (x % 2 == 0)
	{
		return 0;
	} // end if
	
	ht_size_t i = 3;
	for (; i <= floor(sqrt((double)x)); i += 2)
	{
		if (x % i == 0)
		{
			return 0;
		} // end if
	} // end for
	
	return 1;
} // end is_prime()

static ht_size_t
get_next_prime(ht_size_t x)
{
    int i=0;
    while(x >= PRIME_ARRAY[i]){
        i++;
        if(i>(sizeof(PRIME_ARRAY))/(sizeof(ht_size_t))-1){
            break;
        }
    }
    if(i==(sizeof(PRIME_ARRAY))/(sizeof(ht_size_t)))
    {
        x=x+10000;
        while (!is_prime(x))
        {
            x++;
        } // end while
        printf("get one bigger prime...%d\n",x);
        return x;
    }
    return PRIME_ARRAY[i];
} // end get_next_prime()

static int 
ht_resize(ht_t *ht, ht_size_t new_bs)
{
    if (!ht)
	{
		return -1;
	} // end if
	
    ht_t *new_ht = _ht_create(new_bs, ht->data_dtor, ht->prime_finder, ht->hash_func);
	if (!new_ht)
	{
		return -1;
	} // end if
	
	ht_size_t i = 0;
    for (; i < ht->size; i++)
	{
        ht_item_t *curr_item =ht->items[i];
        if (curr_item != NULL)
		{
            int res = ht_tf(new_ht, curr_item);
			if (res == -1)
			{
				return -1;
			} // end if
            ht->items[i]=NULL;
		} // end if
	} // end for

    ht_size_t tmp_bs = ht->base_size;
    ht->base_size = new_ht->base_size;
    new_ht->base_size = tmp_bs;
	
    ht_size_t tmp_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = tmp_size;
	
    ht_size_t tmp_count = ht->count;
    ht->count = new_ht->count;
    new_ht->count = tmp_count;
	
    ht_item_t **tmp_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = tmp_items;

    int res = ht_destroy(new_ht);
    if (res == -1)
    {
        return -1;
    } // end if
    else
    {
        return 0;
    }
} // end ht_resize()

static int
ht_resize_up(ht_t *ht)
{
    if (!ht || ht->base_size == 0)
	{
		return -1;
	} // end if
	
    ht_size_t new_bs = ht->prime_finder(ht->base_size);
	
    return ht_resize(ht, new_bs);
} // end ht_resize_up()

static int
ht_resize_down(ht_t *ht)
{
	if (!ht || ht->base_size == 0)
	{
		return -1;
	} // end if
	
	ht_size_t new_bs = ht->base_size / 2;
	
	return ht_resize(ht, new_bs);
} // end ht_resize_down()

static void 
safe_free(void **pp)
{
	if (pp != NULL && *pp != NULL)
	{
		free(*pp);
        *pp = NULL;
	} // end if
} // end safe_free()
