//////////////////////////////////////////////////////////////////
//
// Unit tests για τον ADT Graph.
// Οποιαδήποτε υλοποίηση οφείλει να περνάει όλα τα tests.
//
//////////////////////////////////////////////////////////////////

#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing

#include "ADTGraph.h"

int compare_vertices(Pointer a, Pointer b) {
	return !(a == b);
}

void test_create() {
	Graph graph = graph_create(compare_vertices, free);
	graph_set_hash_function(graph, hash_pointer);

	TEST_ASSERT(graph != NULL);
	TEST_ASSERT(graph_size(graph) == 0);

	graph_destroy(graph);
}

// Επιστρέφει έναν ακέραιο σε νέα μνήμη με τιμή value
int* create_int(int value) {
	int* p = malloc(sizeof(int));
	*p = value;
	return p;
}

// Βοηθητική συνάρτηση για το ανακάτεμα του πίνακα τιμών
void shuffle(int* array[], int n) {
	for (int i = 0; i < n; i++) {
		int j = i + rand() / (RAND_MAX / (n - i) + 1);
		int* t = array[j];
		array[j] = array[i];
		array[i] = t;
	}
}

void test_insert() {

	Graph graph = graph_create(compare_vertices, free);
	graph_set_hash_function(graph, hash_pointer);

	int N = 1000;
	int** vertex_array = malloc(N * sizeof(*vertex_array));

	for (int i = 0; i < N; i++) {
		vertex_array[i] = create_int(i);
	}

	// Ανακατεύουμε το key_array ώστε να υπάρχει ομοιόμορφη εισαγωγή τιμών
	shuffle(vertex_array, N);
	// Δοκιμάζουμε την insert εισάγοντας κάθε φορά νέους κόμβους
	for (int i = 0; i < N; i++) {
		// Εισαγωγή, δοκιμή και έλεγχος ότι ενημερώθηκε το size
		graph_insert_vertex(graph, vertex_array[i]);
		// Βάζουμε και μια ακμή ανά δύο κορυφές
		if (i) {
			graph_insert_edge(graph, vertex_array[i - 1], vertex_array[i], i);
		}
		TEST_ASSERT(graph_size(graph) == (i + 1));
	}

	// Ελέγχουμε ότι υπάρχουν όλες οι κορυφές
	List vertex_list = graph_get_vertices(graph);
	TEST_ASSERT(list_size(vertex_list) == N);
	for (int i = 0 ; i < N ; i++) {
		TEST_ASSERT(list_find(vertex_list, vertex_array[i], compare_vertices) != NULL);
	}
	list_destroy(vertex_list);

	// Ελέγχουμε ότι μπήκαν σωστά όλες οι ακμές
	List neighb_list = graph_get_adjacent(graph, vertex_array[0]);
	TEST_ASSERT(list_size(neighb_list) == 1);
	TEST_ASSERT(list_node_value(neighb_list, list_first(neighb_list)) == vertex_array[1]);
	list_destroy(neighb_list);
	for (uint i = 1 ; i < N - 1; i++) {
		neighb_list = graph_get_adjacent(graph, vertex_array[i]);
		TEST_ASSERT(list_size(neighb_list) == 2);
		TEST_ASSERT((list_node_value(neighb_list, list_first(neighb_list)) == vertex_array[i + 1] &&
		list_node_value(neighb_list, list_next(neighb_list, list_first(neighb_list))) == vertex_array[i - 1]) ||
		(list_node_value(neighb_list, list_first(neighb_list)) == vertex_array[i - 1] &&
		list_node_value(neighb_list, list_next(neighb_list, list_first(neighb_list))) == vertex_array[i + 1]));
		list_destroy(neighb_list);
	}
	neighb_list = graph_get_adjacent(graph, vertex_array[N - 1]);
	TEST_ASSERT(list_size(neighb_list) == 1);
	TEST_ASSERT(list_node_value(neighb_list, list_first(neighb_list)) == vertex_array[N - 2]);
	list_destroy(neighb_list);

	// Ελέγχουμε ότι είναι σωστά όλα τα βάρη
	for (uint i = 0 ; i < N - 1 ; i++) {
		TEST_ASSERT(graph_get_weight(graph, vertex_array[i], vertex_array[i + 1]) == i + 1);
		TEST_ASSERT(graph_get_weight(graph, vertex_array[i + 1], vertex_array[i]) == i + 1);
	}

	// Ελέγχουμε ότι το βάρος είναι 0 για ίδιες κορυφές
	for (uint i = 0 ; i < N - 1 ; i++) {
		TEST_ASSERT(graph_get_weight(graph, vertex_array[i], vertex_array[i]) == 0);
	}

	// Βάζουμε μερικές ακόμα ακμές για να μην ελέγχουμε μόνο μια εκφυλισμένη λίστα
	for (int i = 0 ; i < N - 2 ; i++) {
		graph_insert_edge(graph, vertex_array[i], vertex_array[i + 2], i + 10);
	}
	// ΕΛέγχουμε ότι μπήκαν σωστά
	for (uint i = 2 ; i < N - 2 ; i++) {
		neighb_list = graph_get_adjacent(graph, vertex_array[i]);
		TEST_ASSERT(list_size(neighb_list) == 4);
		list_destroy(neighb_list);
	}

	graph_destroy(graph);
	free(vertex_array);
}

void test_remove() {

	Graph graph = graph_create(compare_vertices, free);
	graph_set_hash_function(graph, hash_pointer);

	int N = 1000;
	int** vertex_array = malloc(N * sizeof(*vertex_array));

	for (int i = 0; i < N; i++) {
		vertex_array[i] = create_int(i);
	}

	// Ανακατεύουμε το key_array ώστε να υπάρχει ομοιόμορφη εισαγωγή τιμών
	shuffle(vertex_array, N);
	// Δοκιμάζουμε την insert εισάγοντας κάθε φορά νέους κόμβους
	for (int i = 0; i < N; i++) {
		// Εισαγωγή κορυφών και ακμών
		graph_insert_vertex(graph, vertex_array[i]);
		if (i) {
			graph_insert_edge(graph, vertex_array[i - 1], vertex_array[i], i);
		}
	}

	List neighb_list;
	for (int i = N - 1; i >= 0; i--) {
		// Αφαιρούμε κορυφές και ελέγχουμε ότι μειώνεται το μέγεθος
		graph_remove_vertex(graph, vertex_array[i]);
		TEST_ASSERT(graph_size(graph) == (i));

		// Και πως όλος ο υπόλοιπος γράφος παραμένει ανεπηρέαστος
		for (int k = 1 ; k < i - 1; k++) {
			neighb_list = graph_get_adjacent(graph, vertex_array[k]);
			TEST_ASSERT(list_size(neighb_list) == 2);
			TEST_ASSERT((list_node_value(neighb_list, list_first(neighb_list)) == vertex_array[k + 1] &&
			list_node_value(neighb_list, list_next(neighb_list, list_first(neighb_list))) == vertex_array[k - 1]) ||
			(list_node_value(neighb_list, list_first(neighb_list)) == vertex_array[k - 1] &&
			list_node_value(neighb_list, list_next(neighb_list, list_first(neighb_list))) == vertex_array[k + 1]));
			list_destroy(neighb_list);
		}

		for (int k = 0 ; k < i - 1; k++) {
			TEST_ASSERT(graph_get_weight(graph, vertex_array[k], vertex_array[k + 1]) == k + 1);
			TEST_ASSERT(graph_get_weight(graph, vertex_array[k + 1], vertex_array[k]) == k + 1);
		}

		if (i) {
			neighb_list = graph_get_adjacent(graph, vertex_array[i - 1]);
			TEST_ASSERT(list_size(neighb_list) == (i == 1 ? 0 : 1));
			list_destroy(neighb_list);
		}
	}

	graph_destroy(graph);

	graph = graph_create(compare_vertices, free);
	graph_set_hash_function(graph, hash_pointer);

	for (int i = 0; i < N; i++) {
		vertex_array[i] = create_int(i);
	}

	// Ανακατεύουμε το key_array ώστε να υπάρχει ομοιόμορφη εισαγωγή τιμών
	shuffle(vertex_array, N);

	for (int i = 0; i < N; i++) {
		// Εισαγωγή κορυφών και ακμών
		graph_insert_vertex(graph, vertex_array[i]);
		if (i) {
			graph_insert_edge(graph, vertex_array[i - 1], vertex_array[i], i);
		}
	}

	for (int i = 0 ; i  < N - 1; i++) {
		// Ελέγχουμε ότι μαζί με τις κορυφές αφαιρούνται και οι ακμές
		// οι γείτονες δηλαδή δεν "βλέπουν" τις αφαιρεμένες κορυφές
		graph_remove_edge(graph, vertex_array[i], vertex_array[i + 1]);
		neighb_list = graph_get_adjacent(graph, vertex_array[i]);
		TEST_ASSERT(list_size(neighb_list) == 0);
		list_destroy(neighb_list);
	}
	
	graph_destroy(graph);
	free(vertex_array);
}

void test_shortest_path() {
	Graph graph = graph_create(compare_vertices, free);
	graph_set_hash_function(graph, hash_pointer);

	int N = 1000;
	int** vertex_array = malloc(N * sizeof(*vertex_array));

	for (int i = 0; i < N; i++) {
		vertex_array[i] = create_int(i);
	}

	// Ανακατεύουμε το key_array ώστε να υπάρχει ομοιόμορφη εισαγωγή τιμών
	shuffle(vertex_array, N);
	// Δοκιμάζουμε την insert εισάγοντας κάθε φορά νέους κόμβους
	for (int i = 0; i < N; i++) {
		// Εισαγωγή κορυφών και ακμών
		graph_insert_vertex(graph, vertex_array[i]);
		if (i) {
			graph_insert_edge(graph, vertex_array[i - 1], vertex_array[i], i);
		}
	}

	// Τρέχουμε Dijkstra στον γράφο-λίστα και ελέγχουμε πως λειτουργεί
	List path = graph_shortest_path(graph, vertex_array[0], vertex_array[N - 1]);
	TEST_ASSERT(list_size(path) == N);
	int i;
	ListNode node;
	for (i = 0, node = list_first(path) ; i < N ; i++, node = list_next(path, node)) {
		TEST_ASSERT(list_node_value(path, node) == vertex_array[i]);
	}
	list_destroy(path);

	for (int i = 0; i < N - 2; i += 2) {
		// Εισαγωγή επιπλέον ακμών
		graph_insert_edge(graph, vertex_array[i], vertex_array[i + 2], i/2);
	}

	// Ελέγχουμε πώς ακολουθεί το πιο σύντομο μονοπάτι (ανά δύο)
	path = graph_shortest_path(graph, vertex_array[0], vertex_array[N - 1]);
	TEST_ASSERT(list_size(path) == (N + 2)/2);
	for (i = 0, node = list_first(path) ; i < 0 ; i++, node = list_next(path, node)) {
		TEST_ASSERT(list_node_value(path, node) == vertex_array[i]);
	}
	list_destroy(path);

	// Βάζουμε μια ακμή από την αρχή προς το τέλος και ελέγχουμε πως την χρησιμοποιεί
	graph_insert_edge(graph, vertex_array[0], vertex_array[N - 1], 1);
	path = graph_shortest_path(graph, vertex_array[0], vertex_array[N - 1]);
	TEST_ASSERT(list_size(path) == 2);
	TEST_ASSERT((list_node_value(path, list_first(path)) == vertex_array[0]) &&
	(list_node_value(path, list_next(path, list_first(path))) == vertex_array[N - 1]));
	list_destroy(path);

	// Απομονώνουμε τον πρώτο κόμβο και ελέγχουμε πως δεν βρίσκεται μονοπάτι
	graph_remove_edge(graph, vertex_array[0], vertex_array[N - 1]);
	graph_remove_edge(graph, vertex_array[0], vertex_array[1]);
	graph_remove_edge(graph, vertex_array[0], vertex_array[2]);
	path = graph_shortest_path(graph, vertex_array[0], vertex_array[N - 1]);
	TEST_ASSERT(list_size(path) == 0);
	list_destroy(path);
	
	graph_destroy(graph);
	free(vertex_array);
}

// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {

	{ "graph_create", test_create },
	{ "graph_insert", test_insert },
	{ "graph_remove", test_remove },
	{ "graph_shortest_path", test_shortest_path },

	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};