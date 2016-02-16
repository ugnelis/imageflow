#include <glenn/png/png.h>
#include "catch.hpp"
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

#include "imageflow.h"

#include "fastscaling_private.h"
#include "weighting_test_helpers.h"
#include "trim_whitespace.h"
#include "string.h"
#include "lcms2.h"
#include "png.h"
#include "curl/curl.h"
#include "curl/easy.h"

#define ERR(c) REQUIRE_FALSE(Context_print_and_exit_if_err(c))

TEST_CASE ("create tiny graph", "")
{
    Context * c = Context_create();
    flow_graph * g = flow_graph_create(c, 10, 10, 200, 2.0);
    int32_t last;

    last = flow_node_create_canvas(c, &g, -1, Bgra32, 400, 300, 0xFFFFFFFF);
    last = flow_node_create_scale(c, &g, last, 300, 200);
    last = flow_node_create_resource_placeholder(c, &g, last, 0);

    ERR(c);

    REQUIRE(g->edges[0].from == 0);
    REQUIRE(g->edges[0].to == 1);
    REQUIRE(g->edge_count == 2);
    REQUIRE(g->node_count == 3);


    flow_graph_destroy(c, g);
    Context_destroy(c);
}

TEST_CASE ("delete a node from a graph", "")
{
    Context * c = Context_create();
    flow_graph * g = flow_graph_create(c, 10, 10, 200, 2.0);
    int32_t last;


    last = flow_node_create_canvas(c, &g, -1, Bgra32, 400, 300, 0xFFFFFFFF);
    last = flow_node_create_scale(c, &g, last, 300, 200);
    last = flow_node_create_resource_placeholder(c, &g, last, 0);
    ERR(c);

    REQUIRE(g->edges[0].from == 0);
    REQUIRE(g->edges[0].to == 1);
    REQUIRE(g->edges[1].from == 1);
    REQUIRE(g->edges[1].to == 2);
    REQUIRE(g->edge_count == 2);
    REQUIRE(g->node_count == 3);

    flow_node_delete(c,g,last);
    ERR(c);

    REQUIRE(g->edge_count == 1);
    REQUIRE(g->node_count == 2);
    REQUIRE(g->nodes[last].type == flow_ntype_Null);
    REQUIRE(g->nodes[last].info_byte_index == -1);
    REQUIRE(g->nodes[last].info_bytes == 0);
    REQUIRE(g->edges[1].type == flow_edgetype_null);
    REQUIRE(g->edges[1].info_bytes == 0);
    REQUIRE(g->edges[1].info_byte_index == -1);
    REQUIRE(g->edges[1].from == -1);
    REQUIRE(g->edges[1].to == -1);



    flow_graph_destroy(c, g);
    Context_destroy(c);
}


TEST_CASE ("clone an edge", "")
{
    Context * c = Context_create();
    flow_graph * g = flow_graph_create(c, 10, 10, 200, 2.0);
    int32_t last;
    last = flow_node_create_canvas(c, &g, -1, Bgra32, 400, 300, 0xFFFFFFFF);
    last = flow_node_create_scale(c, &g, last, 300, 200);

    ERR(c);

    REQUIRE(g->edges[0].from == 0);
    REQUIRE(g->edges[0].to == 1);
    REQUIRE(g->edge_count == 1);
    REQUIRE(g->node_count == 2);

    flow_edge_duplicate(c,&g, 0);

    ERR(c);

    REQUIRE(g->edge_count == 2);
    REQUIRE(g->node_count == 2);
    REQUIRE(g->edges[1].from == 0);
    REQUIRE(g->edges[1].to == 1);


    flow_graph_destroy(c, g);
    Context_destroy(c);
}


//TODO test paths where adding nodes/edges exceeds the max size



TEST_CASE("execute tiny graph", "")
{

    Context * c = Context_create();
    struct flow_graph *g = nullptr, *g2 = nullptr, *g3 = nullptr;
    struct flow_job *job = nullptr;

    int32_t result_resource_id;
    BitmapBgra * result = nullptr;

    g = flow_graph_create(c, 10, 10, 200, 2.0);
    ERR(c);

    int32_t last;


    last = flow_node_create_canvas(c, &g, -1, Bgra32, 400, 300, 0xFFFFFFFF);
    //last = flow_node_create_scale(c, g, last, 300, 200);
    last = flow_node_create_resource_placeholder(c, &g, last, 0);



    job = flow_job_create(c);
    ERR(c);

    result_resource_id = flow_job_add_bitmap_bgra(c,job, FLOW_OUTPUT, /* graph placeholder index */ 0);




    g2 = flow_job_complete_graph(c,job,g);
    ERR(c);



    g3 = flow_graph_flatten(c,g2, false);
    ERR(c);


    if (!flow_job_execute_graph(c,job,g3)){
        ERR(c);
    }

    REQUIRE(result_resource_id == 2048);
    result = flow_job_get_bitmap_bgra(c, job, result_resource_id);

    ERR(c);


    REQUIRE(result != NULL);
    REQUIRE(result->w == 400);


    BitmapBgra_destroy(c,result);
    flow_job_destroy(c,job);
    flow_graph_destroy(c, g3);
    flow_graph_destroy(c, g2);
    flow_graph_destroy(c, g);
    Context_destroy(c);
}