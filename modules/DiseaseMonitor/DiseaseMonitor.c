///////////////////////////////////////////////////////////////////
//
// Disease Monitor
//
// Module που παρέχει στατιστικά για την εξέλιξη μιας ασθένειας.
//
///////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "DiseaseMonitor.h"
#include "ADTList.h"
#include "ADTMap.h"
#include "ADTSet.h"
#include "ADTPriorityQueue.h"

// Struct που αποθηκεύει μια ασθένεια και το πλήθος κρουσμάτων αυτής.

typedef struct dis_count* DisCount;

struct dis_count {
	String disease;
	int cases;
};

uint hash_dis_country(Pointer value) {
	char total[strlen(((Record) value)->disease) + strlen(((Record) value)->country) + 1];
	strcpy(total, ((Record) value)->disease);
	strcat(total, ((Record) value)->country);
	return hash_string(total);
}

uint hash_disease(Pointer value) {
	return hash_string(((Record) value)->disease);
}

uint hash_country(Pointer value) {
	return hash_string(((Record) value)->country);
}

uint hash_id(Pointer value) {
	return ((Record) value)->id;
}

int compare_record_dates(Pointer a, Pointer b) {
	int result = strcmp(((Record) a)->date, ((Record) b)->date);
	if (result) {
		return result;
	}
	return ((Record) a)->id - ((Record) b)->id;
}

int compare_records_country_dis(Pointer a, Pointer b) {
	int result = strcmp(((Record) a)->country, ((Record) b)->country);
	if (result) {
		return result;
	}
	return strcmp(((Record) a)->disease, ((Record) b)->disease);
}

int compare_countries(Pointer a, Pointer b) {
	return strcmp(((Record) a)->country, ((Record) b)->country);
}

int compare_diseases(Pointer a, Pointer b) {
	return strcmp(((Record) a)->disease, ((Record) b)->disease);
}

int compare_ids(Pointer a, Pointer b) {
	return ((Record) a)->id - ((Record) b)->id;
}

int compare_cases(Pointer a, Pointer b) {
	return ((DisCount) a)->cases - ((DisCount) b)->cases;
}

static Map country_dis_map, id_map, dis_map, country_map, country_to_pq, country_dis_to_pqnode, dis_to_pqnode;
static Set total_set;
static PriorityQueue total_pq;

// Αρχικοποιεί όλες τις δομές του monitor, πρέπει να κληθεί πριν από οποιαδήποτε άλλη κλήση.
// Αν υπήρχαν ήδη δεδομένα τα διαγράφει καλώντας την dm_destroy.

void dm_init() {
	country_dis_map = map_create(compare_records_country_dis, NULL, (DestroyFunc) set_destroy);
	map_set_hash_function(country_dis_map, hash_dis_country);
	dis_map = map_create(compare_diseases, NULL, (DestroyFunc) set_destroy);
	map_set_hash_function(dis_map, hash_disease);
	country_map = map_create(compare_countries, NULL, (DestroyFunc) set_destroy);
	map_set_hash_function(country_map, hash_country);
	id_map =  map_create(compare_ids, NULL, NULL);
	map_set_hash_function(id_map, hash_id);
	country_to_pq = map_create(compare_countries, NULL, (DestroyFunc) pqueue_destroy);
	map_set_hash_function(country_to_pq, hash_country);
	country_dis_to_pqnode = map_create(compare_records_country_dis, NULL, NULL);
	map_set_hash_function(country_dis_to_pqnode, hash_dis_country);
	dis_to_pqnode = map_create(compare_diseases, NULL, NULL);
	map_set_hash_function(dis_to_pqnode, hash_disease);
	total_set = set_create(compare_record_dates, NULL);
	total_pq = pqueue_create(compare_cases, free, NULL);
}

// Καταστρέφει όλες τις δομές του monitor, απελευθερώνοντας την αντίστοιχη
// μνήμη. ΔΕΝ κάνει free τα records, αυτά δημιουργούνται και καταστρέφονται από
// τον χρήστη.

void dm_destroy() {
	map_destroy(country_dis_map);
	map_destroy(id_map);
	map_destroy(dis_map);
	map_destroy(country_map);
	map_destroy(country_to_pq);
	map_destroy(country_dis_to_pqnode);
	map_destroy(dis_to_pqnode);
	set_destroy(total_set);
	pqueue_destroy(total_pq);
}


// Προσθέτει την εγγραφή record στο monitor. Δεν δεσμεύει νέα μνήμη (ούτε
// φτιάχνει αντίγραφα του record), απλά αποθηκεύει τον pointer (η δέσμευση
// μνήμης για τα records είναι ευθύνη του χρήστη). Αν υπάρχει εγγραφή με το ίδιο
// id αντικαθίσταται και επιστρέφεται true, αν όχι επιστρέφεται false.
//
// Οι αλλαγές στα δεδομένα της εγγραφής απαγορεύονται μέχρι να γίνει remove από
// τον monitor.

bool dm_insert_record(Record record) {
	Set dest_set;
	map_insert(id_map, record, record);
	set_insert(total_set, record);
	if ((dest_set = map_find(dis_map, record))) {
		set_insert(dest_set, record);
	}
	else {
		dest_set = set_create(compare_record_dates, NULL);
		map_insert(dis_map, record, dest_set);
		set_insert(dest_set, record);
	}
	if ((dest_set = map_find(country_map, record))) {
		set_insert(dest_set, record);
	}
	else {
		dest_set = set_create(compare_record_dates, NULL);
		map_insert(country_map, record, dest_set);
		set_insert(dest_set, record);
	}

	PriorityQueue dest_pq;
	PriorityQueueNode node;
	DisCount count;
	if ((dest_pq = map_find(country_to_pq, record))) {
		if ((node = map_find(country_dis_to_pqnode, record))) {
			((DisCount) pqueue_node_value(dest_pq, node))->cases++;
			pqueue_update_order(dest_pq, node);
		}
		else {
			count = malloc(sizeof(*count));
			count->disease = record->disease;
			count->cases = 1;
			node = pqueue_insert(dest_pq, count);
			map_insert(country_dis_to_pqnode, record, node);
		}
	}
	else {
		dest_pq = pqueue_create(compare_cases, free, NULL);
		map_insert(country_to_pq, record, dest_pq);
		count = malloc(sizeof(*count));
		count->disease = record->disease;
		count->cases = 1;
		node = pqueue_insert(dest_pq, count);
		map_insert(country_dis_to_pqnode, record, node);
	}

	if ((node = map_find(dis_to_pqnode, record))) {
		((DisCount) pqueue_node_value(total_pq, node))->cases++;
		pqueue_update_order(total_pq, node);
	}
	else {
		count = malloc(sizeof(*count));
		count->disease = record->disease;
		count->cases = 1;
		node = pqueue_insert(total_pq, count);
		map_insert(dis_to_pqnode, record, node);
	}

	if ((dest_set = map_find(country_dis_map, record))) {
		return set_insert(dest_set, record);
	}
	else {
		dest_set = set_create(compare_record_dates, NULL);
		map_insert(country_dis_map, record, dest_set);
		return set_insert(dest_set, record);
	}
}

// Αφαιρεί την εγγραφή με το συγκεκριμένο id από το σύστημα (χωρίς free, είναι
// ευθύνη του χρήστη). Επιστρέφει true αν υπήρχε τέτοια εγγραφή, αλλιώς false.

bool dm_remove_record(int id) {
	Record temp_record = malloc(sizeof(*temp_record));
	temp_record->id = id;
	Record record = map_find(id_map, temp_record);
	if (record == NULL) {
		free(temp_record);
		return false;
	}
	map_remove(id_map, temp_record);
	free(temp_record);
	Set set = map_find(country_dis_map, record);
	set_remove(set, record);
	if (set_size(set) == 0) {
		map_remove(country_dis_map, record);
	}
	set = map_find(dis_map, record);
	set_remove(set, record);
	if (set_size(set) == 0) {
		map_remove(dis_map, record);
	}
	set = map_find(country_map, record);
	set_remove(set, record);
	if (set_size(set) == 0) {
		map_remove(country_map, record);
	}

	PriorityQueue pqueue = map_find(country_to_pq, record);
	PriorityQueueNode node = map_find(country_dis_to_pqnode, record);
	DisCount count = pqueue_node_value(pqueue, node);
	if (count->cases == 1) {
		map_remove(country_dis_to_pqnode, record);
		pqueue_remove_node(pqueue, node);
		if (pqueue_size(pqueue) == 0) {
			map_remove(country_to_pq, record);
		}
	}
	else {
		count->cases--;
		pqueue_update_order(pqueue, node);
	}

	node = map_find(dis_to_pqnode, record);
	count = pqueue_node_value(total_pq, node);
	if (count->cases == 1) {
		map_remove(dis_to_pqnode, record);
		pqueue_remove_node(total_pq, node);
	}
	else {
		count->cases--;
		pqueue_update_order(total_pq, node);
	}

	set_remove(total_set, record);
	return true;
}


// Monitor queries
//
// Στις παρακάτω συναρτήσεις χρησιμοποιούνται τα παρακάτω κριτήρια αναζήτησης εγγραφών:
//   disease:   Μόνο εγγραφές με τη συγκεκριμένη ασθένεια (όλες, αν NULL)
//   country:   Μόνο εγγραφές με τη συγκεκριμένη χώρα (όλες, αν NULL)
//   date_from: Μόνο εγγραφές με ημερομηνία date_from ή μεταγενέστερη (όλες, αν NULL)
//   date_to:   Μόνο εγγραφές με ημερομηνία date_to ή προγενέστερη (όλες, αν NULL)
//
// Οποιοδήποτε από τα κριτήρια μπορεί να είναι NULL, το οποίο σημαίνει "όλες οι
// εγγραφές" (χωρίς φίλτρο για το συγκεκριμένο πεδίο).
//
// Στις συναρτήσεις που επιστρέφουν λίστα, η λίστα δημιουργείται την ώρα της
// κλήσης και είναι ευθύνη του χρήστη να καλέσει τη list_destroy (η οποία θα
// ελευθερώσει μόνο τη λίστα, όχι τα δεδομένα).


// Επιστρέφει λίστα με τα Records που ικανοποιούν τα συγκεκριμένα κριτήρια, σε
// οποιαδήποτε σειρά.

List dm_get_records(String disease, String country, Date date_from, Date date_to) {
	Map searchmap = country_dis_map;
	Set searchset;
	if ((disease != NULL) || (country != NULL)) {
		if (disease == NULL) {
			searchmap = country_map;
		}
		if (country == NULL) {
			searchmap = dis_map;
		}
		Record temp_record = malloc(sizeof(*temp_record));
		temp_record->disease = disease;
		temp_record->country = country;
		searchset = map_find(searchmap, temp_record);
		free(temp_record);
	}
	else {
		searchset = total_set;
	}
	Record record1 = malloc(sizeof(*record1));
	record1->date = date_from;
	record1->id = 0;
	Record record2 = malloc(sizeof(*record2));
	record2->date = date_to;
	record2->id = INT_MAX;
	List list = set_return_from_to(searchset, (date_from != NULL) ? record1 : NULL, (date_to != NULL) ? record2 : NULL);
	free(record1);
	free(record2);
	return list;
}

// Επιστρέφει τον αριθμό εγγραφών που ικανοποιούν τα συγκεκριμένα κριτήρια.

int dm_count_records(String disease, String country, Date date_from, Date date_to) {
	Map searchmap = country_dis_map;
	Set searchset;
	if ((disease != NULL) || (country != NULL)) {
		if (disease == NULL) {
			searchmap = country_map;
		}
		if (country == NULL) {
			searchmap = dis_map;
		}
		Record temp_record = malloc(sizeof(*temp_record));
		temp_record->disease = disease;
		temp_record->country = country;
		searchset = map_find(searchmap, temp_record);
		free(temp_record);
	}
	else {
		searchset = total_set;
	}
	if (searchset == NULL) {
		return 0;
	}
	Record date_record = malloc(sizeof(*date_record));
	date_record->date = date_from;
	date_record->id = 0;
	int before = (date_from != NULL) ? set_count_less_than(searchset, date_record) : 0;
	date_record->date = date_to;
	date_record->id = INT_MAX;
	int after = (date_to != NULL) ? set_count_greater_than(searchset, date_record) : 0;
	free(date_record);
	return set_size(searchset) - before - after;
}

// Επιστρέφει τις k ασθένειες με τις περισσότερες εγγραφές που ικανοποιούν τo
// κριτήριο country (μπορεί να είναι NULL) _ταξινομημένες_ με βάση τον αριθμό
// εγγραφών (πρώτα η ασθένεια με τις περισσότερες).
//
// Πχ η dm_top_diseases(3, "Germany") επιστρέφει τις 3 ασθένειες με τις
// περισσότερες εγγραφές στη Γερμανία. Επιστρέφονται _μόνο_ ασθένειες με
// __τουλάχιστον 1 εγγραφή__ (που ικανοποιεί τα κριτήρια).

List dm_top_diseases(int k, String country) {
	List top_nodes, top_diseases = list_create(NULL);
	PriorityQueue diseases;
	if (country != NULL) {
		Record temp = malloc(sizeof(*temp));
		temp->country = country;
		diseases = map_find(country_to_pq, temp);
		free(temp);
	}
	else {
		diseases = total_pq;
	}
	top_nodes = pqueue_top_k(diseases, k);
	if (list_size(top_nodes)) {
		list_insert_next(top_diseases, LIST_BOF, ((DisCount) list_node_value(top_nodes, list_first(top_nodes)))->disease);
	}
	ListNode node1, node2;
	for (node1 = list_next(top_nodes, list_first(top_nodes)), node2 = list_first(top_diseases);
		node1 != LIST_EOF;
		node1 = list_next(top_nodes, node1), node2 = list_next(top_diseases, node2)) {
			list_insert_next(top_diseases, node2, ((DisCount) list_node_value(top_nodes, node1))->disease);
	}
	list_destroy(top_nodes);
	return top_diseases;
}