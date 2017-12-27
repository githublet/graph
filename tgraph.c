#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "graph.h"

static void safe_free(void **pp)
{
	if (pp != NULL && *pp != NULL)
	{
		free(*pp);
        *pp = NULL;
	} // end if
} // end safe_free()

#define sfree(p) safe_free((void**)(&(p)))
# define process 1
# define testnum 99999
// the data to store at each vertex of the graph
typedef struct
{
	char *str;
	int i;
}test_data_t;


// a user-provided function to destroy the data stored at a vertex
void 
dtor(void *data)
{
	if (!data)
	{
		return;
	} // end if
	sfree(((test_data_t *)data)->str);

	return;
} // end test_data_dtor()
static test_data_t td[testnum];
graph_vertex_t *v[testnum];
int main()
{
	// create graph--------------------------------------------------------------------
    graph_t *g = graph_create(dtor);
    srand((unsigned) time(NULL));
    if (!g)
	{
		printf("err 0\n");
	} // end if
#if process
    memset(v,0,sizeof(v));
    int i;
    for(i=0;i<testnum;i++){
        td[i].i=i;
        td[i].str=malloc(6);
        snprintf(td[i].str,5,"%d",i);
        v[i]=graph_vertex_create(&td[i],td[i].i);
        if(!v[i]){
            printf("err graph_vertex_create %d\n",i);
        }
    }

    int res;
    for(i=0;i<testnum;i++){
        res= graph_add_vertex(g,v[i]);
        if(res==-1){
            printf("err graph_add_vertex %d\n",i);
        }
    }

    for(i=0;i<testnum;i++){
        int src = rand()%testnum;
        int dest = rand()%testnum;
        graph_edge_t * e = graph_edge_create(v[src], v[dest], 0);
        int res = graph_add_edge(g,e);
        if(res==-1){
            printf("err graph_add_edge %d %d\n",src,dest);
        }
    }

    printf("Cycle dector func hasCycle = %d\n",hasCycle(g));
#else
	// create data---------------------------------------------------------------------
	test_data_t td1;
	td1.i = 1;
    td1.str = _strdup("1");
    graph_vertex_t *v1 = graph_vertex_create(&td1, 1);
	
	test_data_t td2;
	td2.i = 2;
    td2.str = _strdup("2");
    graph_vertex_t *v2 = graph_vertex_create(&td2, 2);
	
	test_data_t td3;
	td3.i = 3;
    td3.str = _strdup("3");
    graph_vertex_t *v3 = graph_vertex_create(&td3, 3);
	
	test_data_t td4;
	td4.i = 4;
    td4.str = _strdup("4");
    graph_vertex_t *v4 = graph_vertex_create(&td4, 4);
	
	test_data_t td5;
	td5.i = 5;
    td5.str = _strdup("5");
    graph_vertex_t *v5 = graph_vertex_create(&td5, 5);
	
    test_data_t td6;
    td6.i = 6;
    td6.str = _strdup("6");
    graph_vertex_t *v6 = graph_vertex_create(&td6, 6);

    test_data_t td7;
    td7.i = 7;
    td7.str = _strdup("7");
    graph_vertex_t *v7 = graph_vertex_create(&td7, 7);

    test_data_t td8;
    td8.i = 8;
    td8.str = _strdup("8");
    graph_vertex_t *v8 = graph_vertex_create(&td8, 8);
	//insert data into graph ---------------------------------------------------------
    graph_add_vertex(g, v1);
    graph_add_vertex(g, v2);
    graph_add_vertex(g, v3);
    graph_add_vertex(g, v4);
    graph_add_vertex(g, v5);
    graph_add_vertex(g, v6);
    graph_add_vertex(g, v7);
    graph_add_vertex(g, v8);
	// add edges to the graph--------------------------------------------------------------
	graph_edge_t * e1 = graph_edge_create(v1, v5, 0);	
	graph_edge_t * e2 = graph_edge_create(v5, v3, 0);
	graph_edge_t * e3 = graph_edge_create(v3, v4, 0);
	graph_edge_t * e4 = graph_edge_create(v2, v3, 0);
	graph_edge_t * e5 = graph_edge_create(v1, v3, 0);
    graph_edge_t * e6 = graph_edge_create(v6, v4, 0);
    graph_edge_t * e7 = graph_edge_create(v5, v7, 0);
    graph_edge_t * e8 = graph_edge_create(v2, v8, 0);
    graph_edge_t * e9 = graph_edge_create(v8, v4, 0);

    graph_add_edge(g,e1);
    graph_add_edge(g,e2);
    graph_add_edge(g,e3);
    graph_add_edge(g,e4);
    graph_add_edge(g,e5);
    graph_add_edge(g,e6);
    graph_add_edge(g,e7);
    graph_add_edge(g,e8);
    graph_add_edge(g,e9);
	// test the state of the graph

    printf("num all inedges of v3 is %d\n",graph_vertex_get_all_inedges(g,3)->len);
    printf("num all outedges of v3 is %d\n",graph_vertex_get_all_outedges(g,3)->len);
    printf("num all inedges of v4 is %d\n",graph_vertex_get_all_inedges(g,4)->len);
    printf("num all outedges of v4 is %d\n",graph_vertex_get_all_outedges(g,4)->len);
    printf("num all inedges of v5 is %d\n",graph_vertex_get_all_inedges(g,5)->len);
    printf("num all outedges of v5 is %d\n",graph_vertex_get_all_outedges(g,5)->len);
    printf("Cycle dector func hasCycle = %d\n",hasCycle(g));

    graph_edge_t * e0 = graph_edge_create(v4, v4, 0);
    graph_add_edge(g,e0);
    printf("Cycle dector func hasCycle = %d\n",hasCycle(g));

    graph_edge_t * e10 = graph_edge_create(v4, v7, 0);
    graph_edge_t * e11 = graph_edge_create(v7, v3, 0);
    graph_add_edge(g,e10);
    graph_add_edge(g,e11);

    printf("num all inedges of v3 is %d\n",graph_vertex_get_all_inedges(g,3)->len);
    printf("num all outedges of v3 is %d\n",graph_vertex_get_all_outedges(g,3)->len);
    printf("num all inedges of v4 is %d\n",graph_vertex_get_all_inedges(g,4)->len);
    printf("num all outedges of v4 is %d\n",graph_vertex_get_all_outedges(g,4)->len);
    printf("num all inedges of v5 is %d\n",graph_vertex_get_all_inedges(g,5)->len);
    printf("num all outedges of v5 is %d\n",graph_vertex_get_all_outedges(g,5)->len);
    printf("Cycle dector func hasCycle = %d\n",hasCycle(g));

    graph_remove_vertex(g,3);
    printf("After remove , v5->v3 = %d\n", graph_has_edge(g, 5, 3));
    printf("After remove , v2->v3 = %d\n", graph_has_edge(g, 2, 3));
    printf("After remove , v3->v4 = %d\n", graph_has_edge(g, 3, 4));
	// destroy the graph and all containing data
#endif
    graph_destroy(g);
	return 0;
} // end main()
