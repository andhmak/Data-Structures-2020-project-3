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

// Ένας γράφος αναπαριστάται από τον τύπο Graph

struct graph {
    Map vertex_list_map;    // Map: vertex -> adjacency list
};

// Μια ακμή αναπαριστάται από τον τύπο Edge

typedef struct edge* Edge;

struct edge {
    Pointer neighb_vertex;  // Γειτονική κορυφή
    uint weight;            // Βάρος ακμής
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
    // Αρχικοποιούμε μια λίστα γειτνίασης
    List neighb_list = list_create(free);
    // Την βάζουμε στο map μαζί με την κορυφή
    map_insert(graph->vertex_list_map, vertex, neighb_list);
}

// Επιστρέφει λίστα με όλες τις κορυφές του γράφου. Η λίστα δημιουργείται σε κάθε
// κληση και είναι ευθύνη του χρήστη να κάνει list_destroy.

List graph_get_vertices(Graph graph) {
    // Φτιάχνουμε μια λίστα και αντιγράφουμε σε αυτήν τις κορυφές που βρίσκονται στο map
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
    // Μέσω της λίστα γειτνίασης της κορυφής βρίσκουμε τις λίστες γειτνίασης όλων των γειτονικών κορυφών
    List neighb_list = map_find(graph->vertex_list_map, vertex);
    List neighb_neighb_list;
    for (ListNode listnode = list_first(neighb_list) ; listnode != LIST_EOF ; listnode = list_next(neighb_list, listnode)) {
        neighb_neighb_list = map_find(graph->vertex_list_map, ((Edge)list_node_value(neighb_list, listnode))->neighb_vertex);
        // Από αυτήν ψάχνουμε και αφαιρούμε την ακμή με την κορυφή που θα αφαιρεθεί
        if (((Edge)list_node_value(neighb_neighb_list, list_first(neighb_neighb_list)))->neighb_vertex == vertex) {
            list_remove_next(neighb_neighb_list, LIST_BOF);
            continue;
        }
        for (ListNode listnode2 = list_first(neighb_neighb_list) ; list_next(neighb_neighb_list, listnode2) != LIST_EOF ; listnode2 = list_next(neighb_neighb_list, listnode2)) {
            if (((Edge)list_node_value(neighb_neighb_list, list_next(neighb_neighb_list, listnode2)))->neighb_vertex == vertex) {
                list_remove_next(neighb_neighb_list, listnode2);
                break;
            }
        }
    }
    // Αφαιρούμε την κορυφή
    map_remove(graph->vertex_list_map, vertex);
}

// Προσθέτει μια ακμή με βάρος weight στο γράφο.
// Αν η ακμή υπάρχει ήδη έχει απροσδιόριστη συμπεριφορά.
// (εφόσον ήταν αποδεκτό το έκανα έτσι γιατί σε ορισμένες περιπτώσεις
// βελτιώνει την πολυπλοκότητα αφού δεν διατρέχουμε τις λίστες γειτνίασης
// για να βρούμε αν υπάρχει ήδη ακμή)

void graph_insert_edge(Graph graph, Pointer vertex1, Pointer vertex2, uint weight) {
    // Δημιουργούμε μια ακμή για κάθε κορυφή και τις προσθέτουμε στις αντίστοιχες λίστες γειτνίασης
    Edge edge1 = malloc(sizeof(*edge1)), edge2 = malloc(sizeof(*edge2));
    edge1->neighb_vertex = vertex2;
    edge1->weight = weight;
    list_insert_next((List) map_find(graph->vertex_list_map, vertex1), LIST_BOF, edge1);
    edge2->neighb_vertex = vertex1;
    edge2->weight = weight;
    list_insert_next((List) map_find(graph->vertex_list_map, vertex2), LIST_BOF, edge2);
}

// Αφαιρεί μια ακμή από το γράφο.

void graph_remove_edge(Graph graph, Pointer vertex1, Pointer vertex2) {
    // Διατρέχουμε τις λίστες γειτνίασης και αφαιρούμε τις ακμές της μιας κορυφής προς την άλλη
    List list1 = map_find(graph->vertex_list_map, vertex1);
    List list2 = map_find(graph->vertex_list_map, vertex2);
    ListNode listnode;
    if (((Edge)list_node_value(list1, list_first(list1)))->neighb_vertex == vertex2) {
        list_remove_next(list1, LIST_BOF);
    }
    else {
        for (listnode = list_first(list1) ; list_next(list1, listnode) != LIST_EOF ; listnode = list_next(list1, listnode)) {
            if (((Edge)list_node_value(list1, list_next(list1, listnode)))->neighb_vertex == vertex2) {
                list_remove_next(list1, listnode);
                break;
            }
        }
    }
    if (((Edge)list_node_value(list2, list_first(list2)))->neighb_vertex == vertex1) {
        list_remove_next(list2, LIST_BOF);
    }
    else {
        for (listnode = list_first(list2) ; list_next(list2, listnode) != LIST_EOF ; listnode = list_next(list2, listnode)) {
            if (((Edge)list_node_value(list2, list_next(list1, listnode)))->neighb_vertex == vertex1) {
                list_remove_next(list2, listnode);
                break;
            }
        }
    }
}

// Επιστρέφει το βάρος της ακμής ανάμεσα στις δύο κορυφές, ή UINT_MAX αν δεν είναι γειτονικές.
// Αν είναι η ίδια κορυφή επιστρέφει 0 και αν κάποια κορυφή δεν υπάρχει έχει απροσδιόριστη συμπεριφορά.

uint graph_get_weight(Graph graph, Pointer vertex1, Pointer vertex2) {
    CompareFunc compare = map_get_compare(graph->vertex_list_map);
    if (!compare(vertex1, vertex2)) {
        return 0;
    }
    // Διατρέχουμε την λίστα γειτνίασης της πρώτης κορυφής και ψάχνουμε την ακμή προς την δεύτερη.
    // Επιστρέφουμε το βάρος.
    List neighb_list = map_find(graph->vertex_list_map, vertex1);
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
    // Φτιάχνουμε μια λίστα και αντιγράφουμε σε αυτήν τις κορυφές που βρίσκονται στην λίστα γειτνίασης της κορυφής
    List newlist = list_create(NULL);
    List oldlist = map_find(graph->vertex_list_map, vertex);
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

// Ο τύπος SearchNode χρησιμοποιείται για τον αλγόριθμο του Dijkstra και αποθηκεύει
// μια κορυφή, την προηγούμενή της στο μονοπάτι, τον κόμβο της μέσα στην pqueue,
// την απόσταση από την αρχή προς αυτήν και το αν είναι μέσα στο "ψαγμένο" σύνολο ή όχι

typedef struct search_node* SearchNode;

struct search_node {
    Pointer vertex;             // κορυφή
    SearchNode prev;            // προηγούμενη στο μονοπάτι
    PriorityQueueNode pqnode;   // κόμβος στην pqueue
    uint dist;                  // απόσταση
    bool in;                    // αν είναι μέσα στο σύνολο ή όχι
};

// Συνάρτηση που συγκρίνει αποστάσεις, για την pqueue

int compare_distances(Pointer a, Pointer b) {
    // Δεν γίνεται αφαίρεση για να μην υπάρχει πρόβλημα
    // με την αλλαγή τύπου από uint σε int
    if (((SearchNode) b)->dist > ((SearchNode) a)->dist) {
        return 1;
    }
    else if (((SearchNode) b)->dist < ((SearchNode) a)->dist) {
        return -1;
    }
    else {
        return 0;
    }
}

List graph_shortest_path(Graph graph, Pointer source, Pointer target) {
    List path = list_create(NULL);
    // search_map: map: vertex --> searchnode
    Map search_map = map_create(map_get_compare(graph->vertex_list_map), NULL, free);
    map_set_hash_function(search_map, map_get_hash_function(graph->vertex_list_map));
    SearchNode searchnode;
    // Για κάθε κορυφή αρχικοποιούμε ένα searchnode και τα προσθέτουμε στο search_map
    for (MapNode mapnode = map_first(graph->vertex_list_map) ; mapnode != MAP_EOF ; mapnode = map_next(graph->vertex_list_map, mapnode)) {
        searchnode = malloc(sizeof(*searchnode));
        searchnode->dist = UINT_MAX;
        searchnode->in = false;
        searchnode->prev = NULL;
        searchnode->vertex = map_node_key(graph->vertex_list_map, mapnode);
        searchnode->pqnode = NULL;
        map_insert(search_map, searchnode->vertex, searchnode);
    }
    // Αρχικοποιούμε την dist_pqueue και προσθέτουμε το source με απόσταση 0
    PriorityQueue dist_pqueue = pqueue_create(compare_distances, NULL, NULL);
    (searchnode = map_find(search_map, source))->dist = 0;
    searchnode->pqnode = pqueue_insert(dist_pqueue, searchnode);

    // Κυρίως αλγόριθμος
    List edges;
    uint alt;
    SearchNode neighb;
    while (pqueue_size(dist_pqueue)) {
        // Επιλέγουμε την πιο "κοντινή" κορυφή
        // Αν φτάσουμε στην κορυφή-προορισμό σταματάμε
        if ((searchnode = pqueue_max(dist_pqueue))->vertex == target) {
            break;
        }
        // Την αφαιρούμε από την pqueue
        pqueue_remove_max(dist_pqueue);
        // Την βάζουμε στο σύνολο
        searchnode->in = true;
        // Παίρουμε την λίστα των γειτόνων
        edges = map_find(graph->vertex_list_map, searchnode->vertex);
        // Για κάθε γείτονα
        for (ListNode listnode = list_first(edges) ; listnode != LIST_EOF ; listnode = list_next(edges, listnode)) {
            neighb = map_find(search_map, ((Edge)list_node_value(edges, listnode))->neighb_vertex);
            // Αν δεν είναι μέσα στο σύνολο
            if (neighb->in) {
                continue;
            }
            // Υπολογίζουμε την απόσταση μέσω της κορυφής
            alt = searchnode->dist + ((Edge)list_node_value(edges, listnode))->weight;
            // Αν η απόσταση είναι μικρότερη, την ενημερώνουμε και θέτουμε την κορυφή ως προηγούμενη
            if (alt < neighb->dist) {
                neighb->dist = alt;
                neighb->prev = searchnode;
                if (neighb->pqnode) {
                    pqueue_update_order(dist_pqueue, neighb->pqnode);
                }
                else {
                    neighb->pqnode = pqueue_insert(dist_pqueue, neighb);
                }
            }
        }
    }
    // Δεν χρειαζόμασε άλλο την pqueue
    pqueue_destroy(dist_pqueue);
    // Επιστρέφουμε την λίστα
    if ((searchnode = map_find(search_map, target))->prev == NULL) {
        map_destroy(search_map);
        return path;
    }
    for ( ; searchnode != NULL ; searchnode = searchnode->prev) {
        list_insert_next(path, LIST_BOF, searchnode->vertex);
    }
    map_destroy(search_map);
    return path;
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