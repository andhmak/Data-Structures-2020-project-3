///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Priority Queue μέσω σωρού.
//
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <assert.h>

#include "ADTPriorityQueue.h"
#include "ADTVector.h"			// Η υλοποίηση του PriorityQueue χρησιμοποιεί Vector
#include "ADTList.h"

// Ενα PriorityQueue είναι pointer σε αυτό το struct
struct priority_queue {
	Vector vector;				// Τα δεδομένα, σε Vector ώστε να έχουμε μεταβλητό μέγεθος χωρίς κόπο
	CompareFunc compare;		// Η διάταξη
	DestroyFunc destroy_value;	// Συνάρτηση που καταστρέφει ένα στοιχείο του vector.
};

// Στο vector περνάμε κόμβους και όχι απλά τιμές. Για βελτίωση
// της πολυπλοκότητας αποθηκεύουμε στον κόμβο και την θέση του
// στο vector (one-based)
// Οι τιμές του vector είναι οι κόμβοι της pqueue, και περιέχουν τις τιμές της pqueue

struct priority_queue_node {
	Pointer value;
	int id;
};

// Με την ίδια μεθοδολογία με αυτήν του UsingADTSet_ADTPriorityQueue, επειδή για απλοποίηση
// του αλγορίθμου (και ομοιότητα με την υλοποίηση του μαθήματος) θα γίνει απελευθέρωση μνήμης
// μέσω του vector, χρησιμοποιούνται η συνάρτηση destroy_nodeκαι η εξωτερική μεταβλητή temp_destroy

static DestroyFunc temp_destroy;

static void destroy_node(Pointer node) {
    PriorityQueueNode pnode = (PriorityQueueNode) node;
    // Απελευθερνουμε την τιμή
    if (temp_destroy != NULL) {
        temp_destroy(pnode->value);
    }
    // Απελευθερώνουμε τον κόμβο
    free(pnode);
}

// Βοηθητικές συναρτήσεις ////////////////////////////////////////////////////////////////////////////

// Προσοχή: στην αναπαράσταση ενός complete binary tree με πίνακα, είναι βολικό τα ids των κόμβων να
// ξεκινάνε από το 1 (ρίζα), το οποίο απλοποιεί τις φόρμουλες για εύρεση πατέρα/παιδιών. Οι θέσεις
// ενός vector όμως ξεκινάνε από το 0. Θα μπορούσαμε απλά να αφήσουμε μία θέση κενή, αλλά δεν είναι ανάγκη,
// μπορούμε απλά να αφαιρούμε 1 όταν διαβάζουμε/γράφουμε στο vector. Για απλοποίηση του κώδικα, η
// πρόσβαση στα στοιχεία του vector γίνεται από τις παρακάτω 2 βοηθητικές συναρτήσεις.

// Επιστρέφει την τιμή (κόμβο pqueue) του κόμβου node_id

static Pointer node_value(PriorityQueue pqueue, int node_id) {
	// τα node_ids είναι 1-based, το node_id αποθηκεύεται στη θέση node_id - 1
	return vector_get_at(pqueue->vector, node_id - 1);
}

// Ανταλλάσει τις τιμές των κόμβων node_id1 και node_id2

static void node_swap(PriorityQueue pqueue, int node_id1, int node_id2) {
	// τα node_ids είναι 1-based, το node_id αποθηκεύεται στη θέση node_id - 1
	Pointer value1 = node_value(pqueue, node_id1);
	Pointer value2 = node_value(pqueue, node_id2);
	int temp = ((PriorityQueueNode) value1)->id;
	((PriorityQueueNode) value1)->id = ((PriorityQueueNode) value2)->id;
	((PriorityQueueNode) value2)->id = temp;
	vector_set_at(pqueue->vector, node_id1 - 1, value2);
	vector_set_at(pqueue->vector, node_id2 - 1, value1);
}

// Αποκαθιστά την ιδιότητα του σωρού.
// Πριν: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού, εκτός από
//       τον node_id που μπορεί να είναι _μεγαλύτερος_ από τον πατέρα του.
// Μετά: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού.

static void bubble_up(PriorityQueue pqueue, int node_id) {
	// Αν φτάσαμε στη ρίζα, σταματάμε
	if (node_id == 1)
		return;

	int parent = node_id / 2;		// Ο πατέρας του κόμβου. Τα node_ids είναι 1-based

	// Αν ο πατέρας έχει μικρότερη τιμή από τον κόμβο, swap και συνεχίζουμε αναδρομικά προς τα πάνω
	if (pqueue->compare(((PriorityQueueNode) node_value(pqueue, parent))->value, ((PriorityQueueNode) node_value(pqueue, node_id))->value) < 0) {
		node_swap(pqueue, parent, node_id);
		bubble_up(pqueue, parent);
	}
}

// Αποκαθιστά την ιδιότητα του σωρού.
// Πριν: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού, εκτός από τον
//       node_id που μπορεί να είναι _μικρότερος_ από κάποιο από τα παιδιά του.
// Μετά: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού.

static void bubble_down(PriorityQueue pqueue, int node_id) {
	// βρίσκουμε τα παιδιά του κόμβου (αν δεν υπάρχουν σταματάμε)
	int left_child = 2 * node_id;
	int right_child = left_child + 1;

	int size = pqueue_size(pqueue);
	if (left_child > size)
		return;

	// βρίσκουμε το μέγιστο από τα 2 παιδιά
	int max_child = left_child;
	if (right_child <= size && pqueue->compare(((PriorityQueueNode) node_value(pqueue, left_child))->value, ((PriorityQueueNode) node_value(pqueue, right_child))->value) < 0)
			max_child = right_child;

	// Αν ο κόμβος είναι μικρότερος από το μέγιστο παιδί, swap και συνεχίζουμε προς τα κάτω
	if (pqueue->compare(((PriorityQueueNode) node_value(pqueue, node_id))->value, ((PriorityQueueNode) node_value(pqueue, max_child))->value) < 0) {
		node_swap(pqueue, node_id, max_child);
		bubble_down(pqueue, max_child);
	}
}

// Αρχικοποιεί το σωρό από τα στοιχεία του vector values.

static void heapify(PriorityQueue pqueue, Vector values) {
	int size = vector_size(values);
	PriorityQueueNode pqnode;
	for (int i = 0 ; i < size; i++) {
		// Δημιουργούμε τον κόμβο
		pqnode = malloc(sizeof(*pqnode));
		pqnode->value = vector_get_at(values, i);
		pqnode->id = i + 1;
		// Προσθέτουμε τον κόμβο στο τέλος το σωρού
		vector_insert_last(pqueue->vector, pqnode);
	}
	// καλούμε την bubble_down για κάθε εσωτερικό κόμβο από κάτω προς την ρίζα
	for (int i = vector_size(values)/2 ; i > 0 ; i--) {
		bubble_down(pqueue, i);
	}
}


// Συναρτήσεις του ADTPriorityQueue //////////////////////////////////////////////////

PriorityQueue pqueue_create(CompareFunc compare, DestroyFunc destroy_value, Vector values) {
	assert(compare != NULL);	// LCOV_EXCL_LINE

	PriorityQueue pqueue = malloc(sizeof(*pqueue));
	pqueue->compare = compare;
	pqueue->destroy_value = destroy_value;

	// Δημιουργία του vector που αποθηκεύει τα στοιχεία.
	// ΠΡΟΣΟΧΗ: ΔΕΝ περνάμε την destroy_value στο vector!
	// Αν την περάσουμε θα καλείται όταν κάνουμε swap 2 στοιχεία, το οποίο δεν το επιθυμούμε.
	pqueue->vector = vector_create(0, NULL);

	// Αν values != NULL, αρχικοποιούμε το σωρό.
	if (values != NULL)
		heapify(pqueue, values);

	return pqueue;
}

int pqueue_size(PriorityQueue pqueue) {
	return vector_size(pqueue->vector);
}

Pointer pqueue_max(PriorityQueue pqueue) {
	return ((PriorityQueueNode) node_value(pqueue, 1))->value;		// root
}

PriorityQueueNode pqueue_insert(PriorityQueue pqueue, Pointer value) {
	PriorityQueueNode pqnode = malloc(sizeof(*pqnode));
	pqnode->value = value;
	pqnode->id = vector_size(pqueue->vector) + 1;
	// Προσθέτουμε τον κόμβο στο τέλος το σωρού
	vector_insert_last(pqueue->vector, pqnode);

 	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τον τελευταίο, που μπορεί να είναι
	// μεγαλύτερος από τον πατέρα του. Αρα μπορούμε να επαναφέρουμε την ιδιότητα του σωρού καλώντας
	// τη bubble_up γα τον τελευταίο κόμβο (του οποίου το 1-based id ισούται με το νέο μέγεθος του σωρού).
	bubble_up(pqueue, pqueue_size(pqueue));

	return pqnode;
}

void pqueue_remove_max(PriorityQueue pqueue) {
	int last_node = pqueue_size(pqueue);
	assert(last_node != 0);		// LCOV_EXCL_LINE

	PriorityQueueNode max_node = node_value(pqueue, 1);

	// Destroy την τιμή που αφαιρείται
	if (pqueue->destroy_value != NULL) {
        pqueue->destroy_value(max_node->value);
    }

	// Αντικαθιστούμε τον πρώτο κόμβο με τον τελευταίο και αφαιρούμε τον τελευταίο
	node_swap(pqueue, 1, last_node);
	vector_remove_last(pqueue->vector);

	// Απελευθερώνουμε τον τελευταίο
    free(max_node);

 	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τη νέα ρίζα
 	// που μπορεί να είναι μικρότερη από κάποιο παιδί της. Αρα μπορούμε να
 	// επαναφέρουμε την ιδιότητα του σωρού καλώντας τη bubble_down για τη ρίζα.
	bubble_down(pqueue, 1);
}

DestroyFunc pqueue_set_destroy_value(PriorityQueue pqueue, DestroyFunc destroy_value) {
	DestroyFunc old = pqueue->destroy_value;
	pqueue->destroy_value = destroy_value;
	return old;
}

void pqueue_destroy(PriorityQueue pqueue) {
	temp_destroy = pqueue->destroy_value;
	// Αντί να κάνουμε εμείς destroy τα στοιχεία, είναι απλούστερο να
	// προσθέσουμε τη destroy_value στο vector ώστε να κληθεί κατά το vector_destroy.
	vector_set_destroy_value(pqueue->vector, destroy_node);
	vector_destroy(pqueue->vector);

	free(pqueue);
}



//// Νέες συναρτήσεις για την εργασία 2 //////////////////////////////////////////

Pointer pqueue_node_value(PriorityQueue set, PriorityQueueNode node) {
	return node->value;
}

// Αφαιρεί τον κόμβο node απελευθερώνοντας αυτόν και την την τιμή που περιέχει
void pqueue_remove_node(PriorityQueue pqueue, PriorityQueueNode node) {
	int id = node->id;
	// Αλλάζουμε την θέση του κόμβου που θα αφαιρεθεί με το τελευταίο
	// και τον αφαιρούμε από το τέλος
	node_swap(pqueue, node->id, vector_size(pqueue->vector));
	vector_remove_last(pqueue->vector);
	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τον τελευταίο
 	// κόμβο οου μετακινήθηκε, ο οποίος μπορεί να είναι μικρότερος από κάποιο παιδί του
	// ή μεγαλύτερος από τον πατέρα του. Αρα μπορούμε να επαναφέρουμε την ιδιότητα του
 	// σωρού καλώντας τη bubble_down και την bubble_up για τη τον κόμβο.
	if (vector_size(pqueue->vector) != id - 1) {	// Αν ο κόμβος που αφαιρέθηκε δεν ήταν τελευταίος
		bubble_up(pqueue, id);
		bubble_down(pqueue, id);
	}

	if (pqueue->destroy_value != NULL) {
        pqueue->destroy_value(node->value);
    }
    free(node);
}

// Αφαιρεί τον κόμβο node χωρίς να απελευθερώσει αυτόν και την την τιμή που περιέχει
static void pqueue_remove_node_nofree(PriorityQueue pqueue, PriorityQueueNode node) {
	int id = node->id;
	// Αλλάζουμε την θέση του κόμβου που θα αφαιρεθεί με το τελευταίο
	// και τον αφαιρούμε από το τέλος
	node_swap(pqueue, node->id, vector_size(pqueue->vector));
	vector_remove_last(pqueue->vector);
	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τον τελευταίο
 	// κόμβο οου μετακινήθηκε, ο οποίος μπορεί να είναι μικρότερος από κάποιο παιδί του
	// ή μεγαλύτερος από τον πατέρα του. Αρα μπορούμε να επαναφέρουμε την ιδιότητα του
 	// σωρού καλώντας τη bubble_down και την bubble_up για τη τον κόμβο.
	if (vector_size(pqueue->vector) != id - 1) {	// Αν ο κόμβος που αφαιρέθηκε δεν ήταν τελευταίος
		bubble_up(pqueue, id);
		bubble_down(pqueue, id);
	}
}

// Προσθέτει τον κόμβο node
static void pqueue_insert_node(PriorityQueue pqueue, PriorityQueueNode node) {
	node->id = vector_size(pqueue->vector) + 1;
	// Προσθέτουμε την τιμή στο τέλος το σωρού
	vector_insert_last(pqueue->vector, node);

 	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τον τελευταίο, που μπορεί να είναι
	// μεγαλύτερος από τον πατέρα του. Αρα μπορούμε να επαναφέρουμε την ιδιότητα του σωρού καλώντας
	// τη bubble_up γα τον τελευταίο κόμβο (του οποίου το 1-based id ισούται με το νέο μέγεθος του σωρού).
	bubble_up(pqueue, pqueue_size(pqueue));
}

// Ενημερώνει την pqueue σε περίπτωση αλλαγής του περιεχομένου της τιμής του κόμβου node
void pqueue_update_order(PriorityQueue pqueue, PriorityQueueNode node) {
	// Αφαιρούμε τον κόμβο χωρίς να τον απελευθερώσουμε
	pqueue_remove_node_nofree(pqueue, node);
	// Ξαναπροσθέτουμε τον κόμβο
	pqueue_insert_node(pqueue, node);
}

static PriorityQueueNode pqueue_remove_and_return_max(PriorityQueue pqueue) {
	int last_node = pqueue_size(pqueue);
	assert(last_node != 0);		// LCOV_EXCL_LINE

	PriorityQueueNode max_node = node_value(pqueue, 1);

	// Αντικαθιστούμε τον πρώτο κόμβο με τον τελευταίο και αφαιρούμε τον τελευταίο
	node_swap(pqueue, 1, last_node);
	vector_remove_last(pqueue->vector);

 	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τη νέα ρίζα
 	// που μπορεί να είναι μικρότερη από κάποιο παιδί της. Αρα μπορούμε να
 	// επαναφέρουμε την ιδιότητα του σωρού καλώντας τη bubble_down για τη ρίζα.
	bubble_down(pqueue, 1);

	return max_node;
}

List pqueue_top_k(PriorityQueue pqueue, int k) {
	List top = list_create(NULL);
	Vector removed_nodes = vector_create(0, NULL);
	if (vector_size(pqueue->vector)) {
		list_insert_next(top, LIST_BOF, ((PriorityQueueNode) node_value(pqueue, 1))->value);
		vector_insert_last(removed_nodes, pqueue_remove_and_return_max(pqueue));
	}
	int i;
	ListNode listnode;
	for (i = 0, listnode = list_first(top) ; i < k - 1 ; i++, listnode = list_next(top, listnode)) {
		if (vector_size(pqueue->vector) == 0) {
			break;
		}
		list_insert_next(top, listnode, ((PriorityQueueNode) node_value(pqueue, 1))->value);
		vector_insert_last(removed_nodes, pqueue_remove_and_return_max(pqueue));
	}
	for (i = 0 ; i < vector_size(removed_nodes) ; i++) {
		pqueue_insert_node(pqueue, vector_get_at(removed_nodes, i));
	}
	vector_destroy(removed_nodes);
	return top;
}