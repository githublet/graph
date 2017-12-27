#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "graph.h"

static void
graph_edge_dtor(void *data);

static int
graph_inedge_cmp(void *d1, void *d2);

static int
graph_outedge_cmp(void *d1, void *d2);

static void
graph_vertex_dtor(void *data);

static list_node_t *
graph_get_inedge_node(graph_t *g, int src, int dest);

static list_node_t *
graph_get_outedge_node(graph_t *g,int src, int dest);

static void 
safe_free(void **pp);

static int DFS(list_t* curr_list,ht_t* stack);

#define sfree(p) safe_free((void**)(&(p)))

static void
graph_edge_dtor(void *data)
{
    return graph_edge_destroy((graph_edge_t *)data);
} // end edge_dtor()

static int
graph_inedge_cmp(void *d1, void *d2)
{
    if (!d1 || !(((graph_edge_t *)d1)->src))
	{
		return 0;
	} // end if

    if (((graph_edge_t *)d1)->src->key == *(int *)d2)
	{
		return 1;
	} // end if
	
	return 0;	
} // end edge_cmp()

static int
graph_outedge_cmp(void *d1, void *d2)
{
    if (!d1 || !(((graph_edge_t *)d1)->dest))
	{
		return 0;
	} // end if

    if (((graph_edge_t *)d1)->dest->key == *(int *)d2)
	{
		return 1;
	} // end if
	
	return 0;	
} // end edge_cmp()

graph_vertex_t *
graph_vertex_create(void *data, int key)
{
	graph_vertex_t *v = malloc(sizeof(graph_vertex_t));
	if (!v)
	{
		return NULL;
	} // end if
	
	v->data = data;
    v->key = key;
    v->is_visit=0;
    v->is_exclude=0;
	v->inedges = list_create(graph_edge_dtor, graph_inedge_cmp);
    if (!v->inedges)
    {
        sfree(v);
        return NULL;
    } // end if
	v->edges = list_create(graph_edge_dtor, graph_outedge_cmp);
    if (!v->edges)
    {
        list_destroy(v->inedges);
		sfree(v);
		return NULL;
	} // end if
	
	v->container = NULL;
	
	return v;
} // end graph_vertex_create()

void
graph_vertex_destroy(graph_vertex_t *v)
{
	if (!v)
	{
		return;
	} // end if
	
	if (v->data && v->container && v->container->vertex_data_dtor)
	{
		v->container->vertex_data_dtor(v->data);
	} // end if
	
	list_destroy(v->inedges);
	list_destroy(v->edges);
	
	sfree(v);
	
	return;
} // end graph_vertex_destroy()

static void
graph_vertex_dtor(void *data)
{
	return graph_vertex_destroy((graph_vertex_t *)data);
} // end graph_vertex_dtor()

graph_edge_t *
graph_edge_create(graph_vertex_t *src, graph_vertex_t *dest, graph_edge_weight_t weight)
{
	if (!src || !dest)
	{
		return NULL;
	} // end if
	
	graph_edge_t *e = malloc(sizeof(graph_edge_t));
	if (!e)
	{
		return NULL;
	} // end if
	
	e->src = src;
	e->dest = dest;
	e->weight = weight;
    e->count = 0;
	return e;
} // end graph_edge_create()

void
graph_edge_destroy(graph_edge_t *e)
{
    if(e->count>2||e->count<0){
        printf("error get from graph_edge_destroy()\n");
        return;
    }
	e->count--;
	if(e->count==0)
		sfree(e);
	return;
} // end graph_edge_destroy()

graph_t *
graph_create(data_dtor_func_t dtor)
{
	if (!dtor)
	{
		return NULL;
	} // end if
	
	graph_t *g = malloc(sizeof(graph_t));
	if (!g)
	{
		return NULL;
	} // end if
	
	g->vertices = ht_create(graph_vertex_dtor);
	if (!g->vertices)
	{
		sfree(g);
		return NULL;
	} // end if
	
	g->vertex_data_dtor = dtor;
	
	g->size = 0;
	
	return g;
} // end graph_create()

void
graph_destroy(graph_t *g)
{
	if (!g)
	{
		return;
	} // end if
	
	if (g->vertices)
	{
		ht_destroy(g->vertices);
	} // end if
	
	sfree(g);
	
	return;
} // end graph_destroy()

int
graph_add_vertex(graph_t *g, graph_vertex_t *v)
{
	if (!g || !g->vertices || !v)
	{
		return -1;
	} // end if
	
	
	ht_item_t *i = ht_item_create(v->key, (void *)v);
	if (!i)
	{
		return -1;
	} // end if
	
    int res = ht_add(g->vertices, i);
	if (res == -1)
	{
		
		sfree(i);
		return -1;
	} // end if
	
	v->container = g;
	(g->size)++;
	
	return 0;
} // end graph_add_vertex()

int
graph_has_vertex(graph_t *g, int key)
{
	if (!g || !g->vertices)
	{
		return 0;
	} // end if
	
	void *res = ht_search(g->vertices, key);
	if (!res)
	{
		return 0;
	} // end if
	
	return 1;
} // end graph_has_vertex()

graph_vertex_t *
graph_get_vertex(graph_t *g,int key)
{
	if (!g || !g->vertices)
	{
		return NULL;
	} // end if
	
	return (graph_vertex_t *)ht_search(g->vertices, key);
} // end graph_get_vertex()

const list_t *
graph_get_all_vertices(graph_t *g)
{
	return NULL;
} // end graph_get_all_vertices()

int
graph_remove_vertex(graph_t *g, int key)
{
    if (!g || !g->vertices || !g->vertices->items)
	{
		return -1;
	} // end if

    list_iterator_t *it = list_iterator_create(graph_vertex_get_all_inedges(g,key),HEAD);
    list_node_t *n=NULL;
    while((n=list_iterator_next(it))){
        list_node_t *m=graph_get_outedge_node(g,((graph_edge_t *)(n->data))->src->key,((graph_edge_t *)(n->data))->dest->key);
        if(!m){
            printf("error get from graph_remove_vertex()\n");
            return -1;
        }
        int res = list_remove_and_destroy_node(m);
        if(res == -1){
            return -1;
        }
    }
    it = list_iterator_create(graph_vertex_get_all_outedges(g,key),HEAD);
    while((n=list_iterator_next(it))){
        list_node_t *m=graph_get_inedge_node(g,((graph_edge_t *)(n->data))->src->key,((graph_edge_t *)(n->data))->dest->key);
        if(!m){
            printf("error get from graph_remove_vertex()\n");
            return -1;
        }
        int res = list_remove_and_destroy_node(m);
        if(res == -1){
            return -1;
        }
    }
    list_iterator_destroy(it);

    int res = ht_delete(g->vertices, key);
	if (res == -1)
	{
		return -1;
	} // end if
	
	(g->size)--;
	return 0;
} // end graph_remove_vertex()

int
graph_remove_all_vertices(graph_t *g)
{
	if (!g || !g->vertices || !g->vertices->items)
	{
		return -1;
	} // end if
	
	size_t i = 0;
	ht_item_t *curr_item = NULL;
	for (; i < g->vertices->size; i++)
	{
		curr_item = g->vertices->items[i];
        if (curr_item !=NULL)
		{
			int res = ht_item_destroy(curr_item);
			if (res == -1)
			{
				return -1;
			} // end if
            g->vertices->items[i]=NULL;
		} // end if
	} // end for
	
	g->size = 0;
	return 0;
} // end graph_remove_all_vertices()

int
graph_add_edge(graph_t *g, graph_edge_t *e)
{
	list_node_t *res;
	if (!g || !e || !e->src || !e->dest)
	{
		return -1;
	} // end if
	
	graph_vertex_t *r1 = graph_get_vertex(g, e->src->key);
	graph_vertex_t *r2 = graph_get_vertex(g, e->dest->key);
	
	if (!r1 || !r2)
	{
		return -1;
	} // end if
	
    list_node_t *n = list_find_node(r1->edges,&e->dest->key);
	
	if (n)
	{
		return -1;
	} // end if
	
    list_node_t *in_node = list_node_create((void *)e);
    if (!in_node)
    {
        return -1;
    } // end if
    res = list_push_back(r2->inedges,in_node);
    if (!res)
    {
        sfree(in_node);
        return -1;
    }

    e->count++;

	list_node_t *out_node = list_node_create((void *)e);
	if (!out_node)
	{
		return -1;
	} // end if
	
	res = list_push_back(r1->edges, out_node);
	if (!res)
	{
		sfree(out_node);
		return -1;
	} // end if
	
    e->count++;

    if(e->count>2){
        printf("error get from graph_add_edge()\n");
        return -1;
    }
	return 0;
} // end graph_add_edge()

static list_node_t *
graph_get_inedge_node(graph_t *g, int src, int dest)
{
    if (!g)
	{
		return NULL;
	} // end if
	
	graph_vertex_t *r1 = graph_get_vertex(g, src);
	graph_vertex_t *r2 = graph_get_vertex(g, dest);
	
	if (!r1 || !r2)
	{
		return NULL;
	} // end if

    return list_find_node(r2->inedges, &src);
	
} // end graph_get_inedge_node()

static list_node_t *
graph_get_outedge_node(graph_t *g, int src, int dest)
{
    if (!g)
	{
		return NULL;
	} // end if
	
	graph_vertex_t *r1 = graph_get_vertex(g, src);
	graph_vertex_t *r2 = graph_get_vertex(g, dest);
	
	if (!r1 || !r2)
	{
		return NULL;
	} // end if

    return list_find_node(r1->edges, &dest);
	
} // end graph_get_outedge_node()

int
graph_has_edge(graph_t *g, int src, int dest)
{
    list_node_t *m = graph_get_inedge_node(g, src, dest);
	list_node_t *n = graph_get_outedge_node(g, src, dest);
	
    if (!m||!n)
	{
		return 0;
	} // end if
	
	return 1;
} // end graph_has_edge()

graph_edge_t *
graph_get_edge(graph_t *g, int src, int dest)
{
	list_node_t *n = graph_get_outedge_node(g, src, dest);
	
	if (!n)
	{
		return NULL;
	} // end if
	
	return (graph_edge_t *)n->data;
} // end graph_get_edge()
list_t *
graph_vertex_get_all_inedges(graph_t *g, int key)
{
	graph_vertex_t *v = graph_get_vertex(g, key);
	if (!v)
	{
		return NULL;
	} //add by zt
	return v->inedges;
}
list_t *
graph_vertex_get_all_outedges(graph_t *g, int key)
{
	graph_vertex_t *v = graph_get_vertex(g, key);
	
	if (!v)
	{
		return NULL;
	} // end if
	
	return v->edges;
} // end graph_vertex_get_all_outedges()


const list_t *
graph_vertex_edges_of(graph_t *g, int key)
{
	return NULL;
} // end graph_vertex_edges_of()

const list_t *
graph_get_all_edges(graph_t *g)
{
	return NULL;
} // end graph_get_all_edges()

int
graph_remove_edge(graph_t *g, int src, int dest)
{
	list_node_t *m = graph_get_inedge_node(g, src, dest);
	if(!m)
	{
		return -1;
	}
	list_node_t *n = graph_get_outedge_node(g, src, dest);
	if (!n)
	{
		return -1;
	} // end if
	
	int res=list_remove_and_destroy_node(m);
	if(res==-1)
	{
		return -1;
	}else{
		return list_remove_and_destroy_node(n);
	}
} // end graph_remove_edge()

int
graph_remove_all_edges(graph_t *g)
{
	if (!g || !g->vertices || !g->vertices->items)
	{
		return -1;
	} // end if
	
	size_t i = 0;
	ht_item_t *curr_item = NULL;
	for (; i < g->vertices->size; i++)
	{
		curr_item = g->vertices->items[i];
        if (curr_item != NULL)
		{
			list_t *curr_inlist = ((graph_vertex_t *)curr_item->v)->inedges;
			list_node_t *curr_innode = curr_inlist->head;
			list_node_t *next_innode = NULL;
			
            while((curr_inlist->len))
			{
				next_innode = curr_innode->next;
				
				int res = list_node_destroy(curr_innode);
				if (res == -1)
				{
					return -1;
				} // end if
				
				curr_innode = next_innode;
                curr_inlist->len--;
			} // end while
			
			list_t *curr_list = ((graph_vertex_t *)curr_item->v)->edges;
			list_node_t *curr_node = curr_list->head;
			list_node_t *next_node = NULL;
			
            while((curr_list->len))
			{
				next_node = curr_node->next;
				
				int res = list_node_destroy(curr_node);
				if (res == -1)
				{
					return -1;
				} // end if
				
				curr_node = next_node;
                curr_list->len--;
			} // end while
		} // end if
	} // end for
	
	return 0;
} // end graph_remove_all_edges()

static int DFS(list_t *curr_list, ht_t *stack){
    list_iterator_t *it=list_iterator_create(curr_list,HEAD);
    if(it){
        list_node_t *n = NULL;
        while((n=list_iterator_next(it))){
            if(((graph_edge_t *)(n->data))->dest->is_exclude)
                continue;
            int key = ((graph_edge_t *)(n->data))->dest->key;
            if(ht_crash(stack,key))
                return 1;
            if(DFS(((graph_edge_t *)(n->data))->dest->edges,stack))
                return 1;
            ((graph_edge_t *)(n->data))->dest->is_visit=1;
            ((graph_edge_t *)(n->data))->dest->is_exclude=1;
            ht_delete(stack,key);
        }
    }
    list_iterator_destroy(it);
    return 0;
}

int hasCycle(graph_t *g){
    if(!g||!g->vertices||!g->vertices->items)
        return 0;
    ht_item_t *curr_item=NULL;
    for(size_t i=0;i<g->vertices->size;i++){
        curr_item = g->vertices->items[i];
        if(curr_item!=NULL){
            ((graph_vertex_t *)(curr_item->v))->is_visit=0;
            ((graph_vertex_t *)(curr_item->v))->is_exclude=0;
        }
    }
    for(size_t i=0;i<g->vertices->size;i++){
        curr_item = g->vertices->items[i];
        if(curr_item!=NULL&&((graph_vertex_t *)(curr_item->v))->is_visit==0){
            ht_t *stack=_ht_create(g->vertices->size,g->vertices->data_dtor,g->vertices->prime_finder,g->vertices->hash_func);
            if(DFS(((graph_vertex_t *)(curr_item->v))->edges,stack)){
                ht_destroy(stack);
                return 1;
            }
            ht_destroy(stack);
        }
    }
    return 0;
}

static void 
safe_free(void **pp)
{
	if (pp != NULL && *pp != NULL)
	{
		free(*pp);
		*pp = NULL;
	} // end if
}// end safe_free()
