/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω Hash Table με open addressing (linear probing)
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "ADTMap.h"
#include "ADTList.h"


// Οι κόμβοι του map στην υλοποίηση με hash table, μπορούν να είναι σε 3 διαφορετικές καταστάσεις,
// ώστε αν διαγράψουμε κάποιον κόμβο, αυτός να μην είναι empty, ώστε να μην επηρεάζεται η αναζήτηση
// αλλά ούτε occupied, ώστε η εισαγωγή να μπορεί να το κάνει overwrite.
typedef enum {
	EMPTY, OCCUPIED, DELETED
} State;

// Το μέγεθος του Hash Table ιδανικά θέλουμε να είναι πρώτος αριθμός σύμφωνα με την θεωρία.
// Η παρακάτω λίστα περιέχει πρώτους οι οποίοι έχουν αποδεδιγμένα καλή συμπεριφορά ως μεγέθη.
// Κάθε re-hash θα γίνεται βάσει αυτής της λίστας. Αν χρειάζονται παραπάνω απο 1610612741 στοχεία, τότε σε καθε rehash διπλασιάζουμε το μέγεθος.
int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};

// Χρησιμοποιούμε open addressing, οπότε σύμφωνα με την θεωρία, πρέπει πάντα να διατηρούμε
// τον load factor του  hash table μικρότερο ή ίσο του 0.5, για να έχουμε αποδoτικές πράξεις
#define MAX_LOAD_FACTOR 0.9

// Δομή του κάθε κόμβου που έχει το hash table (με το οποίο υλοιποιούμε το map)
struct map_node {
	Pointer key;		// Το κλειδί που χρησιμοποιείται για να hash-αρουμε
	Pointer value;  	// Η τιμή που αντισοιχίζεται στο παραπάνω κλειδί
	uint pos;
};

// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct map {
	List *list_array;			// Ο πίνακας δεικτών σε λίστες που θα χρησιμοποιήσουμε για το map (remember, φτιάχνουμε ένα hash table με separate chaining)
	int capacity;				// Πόσο χώρο έχουμε δεσμεύσει.
	int size;					// Πόσα στοιχεία έχουμε προσθέσει
	CompareFunc compare;		// Συνάρτηση για σύγκρηση δεικτών, που πρέπει να δίνεται απο τον χρήστη
	HashFunc hash_function;		// Συνάρτηση για να παίρνουμε το hash code του κάθε αντικειμένου.
	DestroyFunc destroy_key;	// Συναρτήσεις που καλούνται όταν διαγράφουμε έναν κόμβο απο το map.
	DestroyFunc destroy_value;
};


Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value) {
	// Δεσμεύουμε κατάλληλα τον χώρο που χρειαζόμαστε για το hash table
	Map map = malloc(sizeof(*map));
	map->capacity = prime_sizes[0];
	map->list_array = malloc(map->capacity * sizeof(List));

	// Αρχικοποιούμε τους κόμβους που έχουμε σαν διαθέσιμους.
	for (int i = 0; i < map->capacity; i++)
		map->list_array[i] = list_create(NULL);	// Destroy θα κάνουμε εμείς και όχι η λίστα

	map->size = 0;
	map->compare = compare;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;

	return map;
}

// Επιστρέφει τον αριθμό των entries του map σε μία χρονική στιγμή.
int map_size(Map map) {
	return map->size;
}

// Συνάρτηση για την επέκταση του Hash Table σε περίπτωση που ο load factor μεγαλώσει πολύ.
static void rehash(Map map) {
	// Αποθήκευση των παλιών δεδομένων
	int old_capacity = map->capacity;
	List *old_list_array = map->list_array;

	// Βρίσκουμε τη νέα χωρητικότητα, διασχίζοντας τη λίστα των πρώτων ώστε να βρούμε τον επόμενο. 
	int prime_no = sizeof(prime_sizes) / sizeof(int);	// το μέγεθος του πίνακα
	for (int i = 0; i < prime_no; i++) {					// LCOV_EXCL_LINE
		if (prime_sizes[i] > old_capacity) {
			map->capacity = prime_sizes[i]; 
			break;
		}
	}
	// Αν έχουμε εξαντλήσει όλους τους πρώτους, διπλασιάζουμε
	if (map->capacity == old_capacity)					// LCOV_EXCL_LINE
		map->capacity *= 2;								// LCOV_EXCL_LINE

	// Δημιουργούμε ένα μεγαλύτερο hash table
	map->list_array = malloc(map->capacity * sizeof(List));
	for (int i = 0; i < map->capacity; i++)
		map->list_array[i] = list_create(NULL);

	// Τοποθετούμε ΜΟΝΟ τα entries που όντως περιέχουν ένα στοιχείο (το rehash είναι και μία ευκαιρία να ξεφορτωθούμε τα deleted nodes)
	map->size = 0;
	for (int i = 0; i < old_capacity; i++) {
		for (ListNode node = list_first(old_list_array[i]) ; node != LIST_EOF ; node = list_next(old_list_array[i], node)) {
			map_insert(map, ((MapNode) list_node_value(old_list_array[i], node))->key, ((MapNode) list_node_value(old_list_array[i], node))->value);////////////////////////
		}
	}

	//Αποδεσμεύουμε τον παλιό πίνακα ώστε να μήν έχουμε leaks
	for (uint i = 0 ; i < old_capacity ; i++) {
		for (ListNode node = list_first(old_list_array[i]) ; node != LIST_EOF ; node = list_next(old_list_array[i], node)) {////////////
			free(list_node_value(old_list_array[i], node));	// free MapNode
		}
		list_destroy(old_list_array[i]);
	}
	free(old_list_array);
}

// Εισαγωγή στο hash table του ζευγαριού (key, item). Αν το key υπάρχει,
// ανανέωση του με ένα νέο value, και η συνάρτηση επιστρέφει true.

void map_insert(Map map, Pointer key, Pointer value) {
	MapNode newnode;

	ListNode node;
	uint pos = map->hash_function(key) % map->capacity;
	List target_list = map->list_array[pos];
	for (node = list_first(target_list) ; node != LIST_EOF ; node = list_next(target_list, node)) {
		if (!map->compare(((MapNode)list_node_value(target_list, node))->key, key)) {
			if (((MapNode)list_node_value(target_list, node))->key != key && map->destroy_key != NULL) {
				map->destroy_key(((MapNode)list_node_value(target_list, node))->key);
			}
			((MapNode)list_node_value(target_list, node))->key = key;
			if (((MapNode)list_node_value(target_list, node))->value != value && map->destroy_value != NULL) {
				map->destroy_value(((MapNode)list_node_value(target_list, node))->value);
			}
			((MapNode)list_node_value(target_list, node))->value = value;
			break;
		}
	}
	if (node == LIST_EOF) {
		newnode = malloc(sizeof(*newnode));
		newnode->key = key;
		newnode->value = value;
		newnode->pos = pos;
		list_insert_next(target_list, LIST_BOF, newnode);
		map->size++;
	}

	// Αν με την νέα εισαγωγή ξεπερνάμε το μέγιστο load factor, πρέπει να κάνουμε rehash
	float load_factor = (float)map->size / map->capacity;
	if (load_factor > MAX_LOAD_FACTOR)
		rehash(map);
}

// Διαργραφή απο το Hash Table του κλειδιού με τιμή key
bool map_remove(Map map, Pointer key) {
	MapNode node = map_find_node(map, key);
	if (node == MAP_EOF)
		return false;

	List node_parent = map->list_array[node->pos];
	if (list_size(node_parent) == 1) {
		list_remove_next(node_parent, LIST_BOF);
	}
	else {
		for (ListNode listnode = list_first(node_parent) ; list_next(node_parent, listnode) != NULL ; listnode = list_next(node_parent, listnode)) {
			if (((MapNode)list_node_value(node_parent, list_next(node_parent, listnode))) == node) {
				list_remove_next(node_parent, listnode);
			}
		}
	}

	// destroy
	if (map->destroy_key != NULL) {
		map->destroy_key(node->key);
	}
	if (map->destroy_value != NULL) {
		map->destroy_value(node->value);
	}

	free(node);

	map->size--;

	return true;
}

// Αναζήτηση στο map, με σκοπό να επιστραφεί το value του κλειδιού που περνάμε σαν όρισμα.

Pointer map_find(Map map, Pointer key) {
	MapNode node = map_find_node(map, key);
	if (node != MAP_EOF)
		return node->value;
	else
		return NULL;
}


DestroyFunc map_set_destroy_key(Map map, DestroyFunc destroy_key) {
	DestroyFunc old = map->destroy_key;
	map->destroy_key = destroy_key;
	return old;
}

DestroyFunc map_set_destroy_value(Map map, DestroyFunc destroy_value) {
	DestroyFunc old = map->destroy_value;
	map->destroy_value = destroy_value;
	return old;
}

// Απελευθέρωση μνήμης που δεσμεύει το map
void map_destroy(Map map) {
	for (int i = 0; i < map->capacity; i++) {
		for (ListNode node = list_first(map->list_array[i]) ; node != LIST_EOF ; node = list_next(map->list_array[i], node)) {
			if (map->destroy_key != NULL) {
				map->destroy_key(((MapNode) list_node_value(map->list_array[i], node))->key);
			}
			if (map->destroy_value != NULL) {
				map->destroy_value(((MapNode)list_node_value(map->list_array[i], node))->value);
			}
			free(list_node_value(map->list_array[i], node));
		}
		list_destroy(map->list_array[i]);
	}

	free(map->list_array);
	free(map);
}

/////////////////////// Διάσχιση του map μέσω κόμβων ///////////////////////////

MapNode map_first(Map map) {
	//Ξεκινάμε την επανάληψή μας απο το 1ο στοιχείο, μέχρι να βρούμε κάτι όντως τοποθετημένο
	for (int i = 0; i < map->capacity; i++) {
		for (ListNode node = list_first(map->list_array[i]) ; node != LIST_EOF ; node = list_next(map->list_array[i], node)) {
			return list_node_value(map->list_array[i], node);
		}
	}

	return MAP_EOF;
}

MapNode map_next(Map map, MapNode node) {
	for (ListNode listnode = list_first(map->list_array[node->pos]) ; listnode != LIST_EOF ; listnode = list_next(map->list_array[node->pos], listnode)) {
		if (list_node_value(map->list_array[node->pos], listnode) == node) {
			if (list_next(map->list_array[node->pos], listnode) != NULL) {
				return list_node_value(map->list_array[node->pos], list_next(map->list_array[node->pos], listnode));
			}
			break;
		}
	}
	for (uint i = node->pos + 1 ; i < map->capacity ; i++) {
		if (list_size(map->list_array[i])) {
			return list_node_value(map->list_array[i], list_first(map->list_array[i]));
		}
	}

	return MAP_EOF;
}

Pointer map_node_key(Map map, MapNode node) {
	return node->key;
}

Pointer map_node_value(Map map, MapNode node) {
	return node->value;
}

MapNode map_find_node(Map map, Pointer key) {
	List target_list = map->list_array[map->hash_function(key) % map->capacity];
	for (ListNode listnode = list_first(target_list) ; listnode != LIST_EOF ; listnode = list_next(target_list, listnode)) {
		if (((MapNode) list_node_value(target_list, listnode))->key == key) {
			return list_node_value(target_list, listnode);
		}
	}

	return MAP_EOF;
}

// Αρχικοποίηση της συνάρτησης κατακερματισμού του συγκεκριμένου map.
void map_set_hash_function(Map map, HashFunc func) {
	map->hash_function = func;
}

uint hash_string(Pointer value) {
	// djb2 hash function, απλή, γρήγορη, και σε γενικές γραμμές αποδοτική
    uint hash = 5381;
    for (char* s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;			// hash = (hash * 33) + *s. Το foo << 5 είναι γρηγορότερη εκδοχή του foo * 32.
    return hash;
}

uint hash_int(Pointer value) {
	return *(int*)value;
}

uint hash_pointer(Pointer value) {
	return (size_t)value;				// cast σε sizt_t, που έχει το ίδιο μήκος με έναν pointer
}