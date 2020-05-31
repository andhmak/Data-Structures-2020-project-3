///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Graph μέσω λιστών γειτνίασης.
//
///////////////////////////////////////////////////////////

#include "ADTGraph.h"
#include "common_types.h"
#include "ADTList.h"			// Ορισμένες συναρτήσεις επιστρέφουν λίστες
#include "ADTPriorityQueue.h"
#include <stdlib.h>
#include <limits.h>

// Ενα γράφος αναπαριστάται από τον τύπο Graph

typedef struct graph* Graph;

struct graph {
    Map vertex_list_map;
};

typedef struct edge* Edge;

struct edge {
    Pointer neighb_vertex;
    int weight;
};

// Δημιουργεί και επιστρέφει ένα γράφο, στον οποίο τα στοιχεία (οι κορυφές)
// συγκρίνονται με βάση τη συνάρτηση compare. Αν destroy_vertex != NULL, τότε
// καλείται destroy_vertex(vertex) κάθε φορά που αφαιρείται μια κορυφή.

Graph graph_create(CompareFunc compare, DestroyFunc destroy_vertex) {
    Graph graph = malloc(sizeof(*graph));
    graph->vertex_list_map = map_create(compare, destroy_vertex, (DestroyFunc) list_destroy);
    return graph;
}

// Επιστρέφει τον αριθμό στοιχείων (κορυφών) που περιέχει ο γράφος graph.

int graph_size(Graph graph) {
    return map_size(graph->vertex_list_map);
}

// Προσθέτει μια κορυφή στο γράφο.

void graph_insert_vertex(Graph graph, Pointer vertex) {
    List neighb_list = list_create(free);
    map_insert(graph->vertex_list_map, vertex, neighb_list);
}

// Επιστρέφει λίστα με όλες τις κορυφές του γράφου. Η λίστα δημιουργείται σε κάθε
// κληση και είναι ευθύνη του χρήστη να κάνει list_destroy.

List graph_get_vertices(Graph graph) {
    List newlist = list_create(NULL);
    if (map_size(graph->vertex_list_map) == 0) {
        return newlist;
    }
    for (MapNode node = map_first(graph->vertex_list_map) ; node != MAP_EOF ; node = map_next(graph->vertex_list_map, node)) {
        list_insert_next(newlist, LIST_BOF, map_node_key(graph->vertex_list_map, node));
    }
    return newlist;
}

// Διαγράφει μια κορυφή από τον γράφο (αν υπάρχουν ακμές διαγράφονται επίσης).

void graph_remove_vertex(Graph graph, Pointer vertex) {
    List neighb_list = map_node_value(graph->vertex_list_map, map_find_node(graph->vertex_list_map, vertex));
    int a = 1;
    List neighb_neighb_list;
    for (ListNode listnode = list_first(neighb_list) ; listnode != LIST_EOF ; listnode = list_next(neighb_list, listnode)) {
        neighb_neighb_list = map_node_value(graph->vertex_list_map, map_find_node(graph->vertex_list_map, ((Edge)list_node_value(neighb_list, listnode))->neighb_vertex));
        if (((Edge)list_node_value(neighb_neighb_list, list_first(neighb_neighb_list)))->neighb_vertex == vertex) {
            list_remove_next(neighb_neighb_list, LIST_BOF);
            a=0;
            continue;
        }
        for (ListNode listnode2 = list_first(neighb_neighb_list) ; list_next(neighb_neighb_list, listnode2) != LIST_EOF ; listnode2 = list_next(neighb_neighb_list, listnode2)) {
            if (((Edge)list_node_value(neighb_neighb_list, list_next(neighb_neighb_list, listnode2)))->neighb_vertex == vertex) {
                list_remove_next(neighb_neighb_list, listnode2);
                a=0;
                break;
            }
        }
    }
    if (list_size(neighb_list) == 0) {
        a = 0;
    }
    if (a) {
        exit(1);
    }
    map_remove(graph->vertex_list_map, vertex);
}

// Προσθέτει μια ακμή με βάρος weight στο γράφο.

void graph_insert_edge(Graph graph, Pointer vertex1, Pointer vertex2, uint weight) {
    Edge edge1 = malloc(sizeof(*edge1)), edge2 = malloc(sizeof(*edge2));
    edge1->neighb_vertex = vertex2;
    edge1->weight = weight;
    list_insert_next((List) map_node_value(graph->vertex_list_map, map_find_node(graph->vertex_list_map, vertex1)), LIST_BOF, edge1);
    edge2->neighb_vertex = vertex1;
    edge2->weight = weight;
    list_insert_next((List) map_node_value(graph->vertex_list_map, map_find_node(graph->vertex_list_map, vertex2)), LIST_BOF, edge2);
}

// Αφαιρεί μια ακμή από το γράφο.

void graph_remove_edge(Graph graph, Pointer vertex1, Pointer vertex2) {
    List list1 = map_node_value(graph->vertex_list_map, map_find_node(graph->vertex_list_map, vertex1));
    List list2 = map_node_value(graph->vertex_list_map, map_find_node(graph->vertex_list_map, vertex2));
    ListNode listnode;
    int a = 1;
    if (((Edge)list_node_value(list1, list_first(list1)))->neighb_vertex == vertex2) {
        list_remove_next(list1, LIST_BOF);
        a = 0;
    }
    else {
        for (listnode = list_first(list1) ; list_next(list1, listnode) != LIST_EOF ; listnode = list_next(list1, listnode)) {
            if (((Edge)list_node_value(list1, list_next(list1, listnode)))->neighb_vertex == vertex2) {
                list_remove_next(list1, listnode);
                a = 0;
                break;
            }
        }
    }
    if (a) {
        exit(3);
    }
    a = 1;
    if (((Edge)list_node_value(list2, list_first(list2)))->neighb_vertex == vertex1) {
        list_remove_next(list2, LIST_BOF);
        a = 0;
    }
    else {
        for (listnode = list_first(list2) ; list_next(list2, listnode) != LIST_EOF ; listnode = list_next(list2, listnode)) {
            if (((Edge)list_node_value(list2, list_next(list1, listnode)))->neighb_vertex == vertex1) {
                list_remove_next(list2, listnode);
                a = 0;
                break;
            }
        }
    }
    if (a) {
        exit(4);
    }
}

// Επιστρέφει το βάρος της ακμής ανάμεσα στις δύο κορυφές, ή UINT_MAX αν δεν είναι γειτονικές.

uint graph_get_weight(Graph graph, Pointer vertex1, Pointer vertex2) {
    List neighb_list = map_node_value(graph->vertex_list_map, map_find_node(graph->vertex_list_map, vertex1));
    for (ListNode listnode = list_first(neighb_list) ; listnode != LIST_EOF ; listnode = list_next(neighb_list, listnode)) {
        if (((Edge)list_node_value(neighb_list, listnode))->neighb_vertex == vertex2) {
            return ((Edge)list_node_value(neighb_list, listnode))->weight;
        }
    }
    return UINT_MAX;
}

// Επιστρέφει λίστα με τους γείτονες μιας κορυφής. Η λίστα δημιουργείται σε κάθε
// κληση και είναι ευθύνη του χρήστη να κάνει list_destroy.

List graph_get_adjacent(Graph graph, Pointer vertex) {
    List newlist = list_create(NULL);
    List oldlist = map_node_value(graph->vertex_list_map, map_find_node(graph->vertex_list_map, vertex));
    if (list_size(oldlist) == 0) {
        return newlist;
    }
    ListNode oldnode = list_first(oldlist), newnode;
    list_insert_next(newlist, LIST_BOF, ((Edge)list_node_value(oldlist, oldnode))->neighb_vertex);
    for (newnode = list_first(newlist), oldnode = list_next(oldlist, oldnode);
        oldnode != LIST_EOF;
        newnode = list_next(newlist, newnode), oldnode = list_next(oldlist, oldnode))
    {
        list_insert_next(newlist, newnode, ((Edge)list_node_value(oldlist, oldnode))->neighb_vertex);
    }
    return newlist;
}

// Επιστρέφει (σε λίστα) το συντομότερο μονοπάτι ανάμεσα στις κορυφές source και
// target, ή κενή λίστα αν δεν υπάρχει κανένα μονοπάτι. Η λίστα δημιουργείται σε
// κάθε κληση και είναι ευθύνη του χρήστη να κάνει list_destroy.

List graph_shortest_path(Graph graph, Pointer source, Pointer target) {
    List list = list_create(NULL);
    
    return list;
}

// Ελευθερώνει όλη τη μνήμη που δεσμεύει ο γράφος.
// Οποιαδήποτε λειτουργία πάνω στο γράφο μετά το destroy είναι μη ορισμένη.

void graph_destroy(Graph graph) {
    map_destroy(graph->vertex_list_map);
    free(graph);
}



//// Για την περίπτωση που ο γράφος χρησιμοποιεί πίνακα κατακερματισμού

#include "ADTMap.h"	// for HashFunc type

// Ορίζει τη συνάρτηση κατακερματισμού hash_func για το συγκεκριμένο γράφο.
// Πρέπει να κληθεί μετά την graph_create και πριν από οποιαδήποτε άλλη συνάρτηση.

void graph_set_hash_function(Graph graph, HashFunc hash_func) {
    map_set_hash_function(graph->vertex_list_map, hash_func);
}