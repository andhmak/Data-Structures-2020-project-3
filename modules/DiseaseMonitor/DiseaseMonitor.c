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

typedef struct dis_cases* DisCases;

struct dis_cases {
	String disease;
	int cases;
};

// Hash function που παίρνει υπ' όψιν την χώρα και την ασθένεια ενός κρούσματος

static uint hash_dis_country(Pointer value) {
	char total[strlen(((Record) value)->disease) + strlen(((Record) value)->country) + 1];
	strcpy(total, ((Record) value)->disease);
	strcat(total, ((Record) value)->country);
	return hash_string(total);
}

// Hash function που παίρνει υπ' όψιν την ασθένεια ενός κρούσματος

static uint hash_disease(Pointer value) {
	return hash_string(((Record) value)->disease);
}

// Hash function που παίρνει υπ' όψιν την χώρα ενός κρούσματος

static uint hash_country(Pointer value) {
	return hash_string(((Record) value)->country);
}

// Hash function που παίρνει υπ' όψιν το id ενός κρούσματος

static uint hash_id(Pointer value) {
	return ((Record) value)->id;
}

// Συνάρτηση σύγκρισης κρουσμάτων ως προς την ημερομηνία τους.
// Ισοδύναμα θεωρούνται μόνο όσα έχουν ίδιο id, άρα αν δύο διαφορετικά
// έχουν ίδια ημερομηνία κατατάσσονται σε αύξουσα σειρά id

static int compare_record_dates(Pointer a, Pointer b) {
	int result = strcmp(((Record) a)->date, ((Record) b)->date);
	if (result) {
		return result;
	}
	return ((Record) a)->id - ((Record) b)->id;
}

// Συνάρτηση σύγκρισης κρουσμάτων ως προς την χώρα και την ασθένειά τους.
// Μας ενδιαφέρει ουσιαστικά μόνο η περίπτωση της ισοδυναμίας.

static int compare_records_country_dis(Pointer a, Pointer b) {
	int result = strcmp(((Record) a)->country, ((Record) b)->country);
	if (result) {
		return result;
	}
	return strcmp(((Record) a)->disease, ((Record) b)->disease);
}

// Συνάρτηση σύγκρισης κρουσμάτων ως προς την χώρα τους.
// Μας ενδιαφέρει ουσιαστικά μόνο η περίπτωση της ισοδυναμίας.

static int compare_countries(Pointer a, Pointer b) {
	return strcmp(((Record) a)->country, ((Record) b)->country);
}

// Συνάρτηση σύγκρισης κρουσμάτων ως προς την ασθένειά τους.
// Μας ενδιαφέρει ουσιαστικά μόνο η περίπτωση της ισοδυναμίας.

static int compare_diseases(Pointer a, Pointer b) {
	return strcmp(((Record) a)->disease, ((Record) b)->disease);
}

// Συνάρτηση σύγκρισης κρουσμάτων ως προς το id τους.
// Μας ενδιαφέρει ουσιαστικά μόνο η περίπτωση της ισοδυναμίας.

static int compare_ids(Pointer a, Pointer b) {
	return ((Record) a)->id - ((Record) b)->id;
}

// Συνάρτηση σύγκρισης DiseaseCases ως προς τον αριθμό των κρουσματάτων τους.

static int compare_cases(Pointer a, Pointer b) {
	return ((DisCases) a)->cases - ((DisCases) b)->cases;
}

// Οι map country_map, dis_map, country_dis_map οδηγούν από ένα record (key) στο set (value) από records, κατατεταγμένα με την ημερομηνία τους,
// που μοιράζονται την ίδια χώρα, ασθένεια, ή και τα δύο, αντίστοιχα.
// Ο id_map οδηγεί από ένα record με ένα συγκεκριμένο id στο record με το ίδιο id που είναι αποθηκευμένο στο disease monitor.
// Η country_to_pq οδηγεί από μια χώρα (key) στην pqueue (value) από DisCases, δηλαδή από ασθένειες κατατεταγμένες με τον
// αριθμό των κρουσμάτων τους, για αυτήν την χώρα.
// Η country_dis_to_pqnode οδηγεί από ένα record (key) στον κόμβο της pqueue για αυτήν την χώρα (value), ο οποίος
// αναπαριστά αυτήν την ασθένεια.
// Το total_set είναι ένα σύνολο που περιέχει όλα τα records κατατεταγμένα με την ημερομηνία τους.
// Η total_pq είναι μια pqueue που περιέχει όλες τις ασθένειες κατατεταγμένες σύμφωνα με τον αριθμό των κρουσμάτων τους,
// ανεξάρτητα από την χώρα.
// Ο dis_to_pqnode οδηγεί από ένα record (key) με μια συγκεκριμένη ασθένεια στον κόμβο της total_pq που αντιπροσωεύει αυτήν την ασθένεια.

static Map country_map, dis_map, country_dis_map, id_map, country_to_pq, country_dis_to_pqnode, dis_to_pqnode;
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

	country_to_pq = map_create((CompareFunc) strcmp, NULL, (DestroyFunc) pqueue_destroy);
	map_set_hash_function(country_to_pq, hash_string);

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
	bool removed = false;

	// Αν υπάρχει εγγραφή με αυτό το id την αφαιρούμε και σημειώνουμε πως υπήρχε
	if (dm_remove_record(record->id)) {
		removed = true;
	}

	// Προσθέτουμε την νέα εγγραφή
	Set dest_set;

	// Προσθέτουμε το record στο id_map για να το βρίσκουμε μετά από το id του
	map_insert(id_map, record, record);

	// Το προσθέτουμε στο συνολικό σύνολο
	set_insert(total_set, record);

	// Το προσθέτουμε στο σύνολο που καθορίζεται από την ασθένειά του
	if ((dest_set = map_find(dis_map, record))) {
		set_insert(dest_set, record);
	}
	// Αν δεν υπάρχει τέτοιο σύνολο το δημιουργούμε
	else {
		dest_set = set_create(compare_record_dates, NULL);
		map_insert(dis_map, record, dest_set);
		set_insert(dest_set, record);
	}

	// Το προσθέτουμε στο σύνολο που καθορίζεται από την χώρα του
	if ((dest_set = map_find(country_map, record))) {
		set_insert(dest_set, record);
	}
	else {
		dest_set = set_create(compare_record_dates, NULL);
		map_insert(country_map, record, dest_set);
		set_insert(dest_set, record);
	}

	// Το προσθέτουμε στην pqueue της χώρας του
	PriorityQueue dest_pq;
	PriorityQueueNode node;
	DisCases count;
	// Αν υπάρχει pqueue το προσθέτουμε εκεί
	if ((dest_pq = map_find(country_to_pq, record->country))) {
		// ΑΝ υπάρχει ο κόμβος τον ενημερώνουμε
		if ((node = map_find(country_dis_to_pqnode, record))) {
			((DisCases) pqueue_node_value(dest_pq, node))->cases++;
			pqueue_update_order(dest_pq, node);
		}
		// ΑΛλιώς τον δημιουργούμε, και τον αποθηκεύουμε στο αντίστοιχο map
		else {
			count = malloc(sizeof(*count));
			count->disease = record->disease;
			count->cases = 1;
			node = pqueue_insert(dest_pq, count);
			map_insert(country_dis_to_pqnode, record, node);
		}
	}
	// Αλλιώς την δημιουργούμε
	else {
		dest_pq = pqueue_create(compare_cases, free, NULL);
		map_insert(country_to_pq, record->country, dest_pq);
		count = malloc(sizeof(*count));
		count->disease = record->disease;
		count->cases = 1;
		node = pqueue_insert(dest_pq, count);
		map_insert(country_dis_to_pqnode, record, node);
	}

	// Το προσθέτουμε στην συνολική pqueue
	if ((node = map_find(dis_to_pqnode, record))) {
		((DisCases) pqueue_node_value(total_pq, node))->cases++;
		pqueue_update_order(total_pq, node);
	}
	else {
		count = malloc(sizeof(*count));
		count->disease = record->disease;
		count->cases = 1;
		node = pqueue_insert(total_pq, count);
		map_insert(dis_to_pqnode, record, node);
	}

	// Το προσθέτουμε στο σύνολο που καθορίζεται από την ασθένειά και την χώρα του
	if ((dest_set = map_find(country_dis_map, record))) {
		set_insert(dest_set, record);
	}
	else {
		dest_set = set_create(compare_record_dates, NULL);
		map_insert(country_dis_map, record, dest_set);
		set_insert(dest_set, record);
	}

	// Επιστρέφουμε αν αφαιρέθηκε άλλη εγγραφή ή όχι
	return removed;
}

// Αφαιρεί την εγγραφή με το συγκεκριμένο id από το σύστημα (χωρίς free, είναι
// ευθύνη του χρήστη). Επιστρέφει true αν υπήρχε τέτοια εγγραφή, αλλιώς false.

bool dm_remove_record(int id) {
	// Δημιουργούμε ένα προσωρινό record με το δοσμένο id για να
	// βρούμε το record με αυτό το id που έχουμε αποθηκεύσει
	Record temp_record = malloc(sizeof(*temp_record));
	temp_record->id = id;
	Record record = map_find(id_map, temp_record);

	// Αν δεν υπάρχει επιστρέφουμε false
	if (record == NULL) {
		free(temp_record);
		return false;
	}

	// Αλλιώς το αφαιρούμε από το id_map
	map_remove(id_map, temp_record);
	free(temp_record);
	// Από όλα τα sets
	Set set = map_find(country_dis_map, record);
	set_remove(set, record);
	// Τα οποία καταστρέφονται αν μείνουν κενά
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

	// Από την pqueue της χώρας του
	PriorityQueue pqueue = map_find(country_to_pq, record->country);
	PriorityQueueNode node = map_find(country_dis_to_pqnode, record);
	DisCases count = pqueue_node_value(pqueue, node);
	// Ενημερώνεται το node και αν μείνει κενό αφαιρείται
	if (count->cases == 1) {
		map_remove(country_dis_to_pqnode, record);
		pqueue_remove_node(pqueue, node);
		// Και αν μείνει κενή και η pqueue καταστρέφεται
		if (pqueue_size(pqueue) == 0) {
			map_remove(country_to_pq, record->country);
		}
	}
	else {
		count->cases--;
		pqueue_update_order(pqueue, node);
	}

	// Από την συνολική pqueue
	node = map_find(dis_to_pqnode, record);
	count = pqueue_node_value(total_pq, node);
	// Ενημερώνεται ή αφαιρείται ο κόμβος
	if (count->cases == 1) {
		map_remove(dis_to_pqnode, record);
		pqueue_remove_node(total_pq, node);
	}
	else {
		count->cases--;
		pqueue_update_order(total_pq, node);
	}

	// Τέλος αφιρείται από το συνολικό σύνολο κρουσμάτων
	set_remove(total_set, record);

	// Το record αφαιρέθηκε επιτυχώς
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
	// Επιλέγουμε το map ανάλογα με το πόσες πληροφορίες έχουμε για την χώρα
	// και μέσα από από αυτό βρίσκουμε το κατάλληλο set
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

	// Αν δεν βρούμε τέτοιο set τότε δεν υπάρχουν κατάλληλα
	// records και επιστρέφουμε κενή λίστα
	if (searchset == NULL) {
		return list_create(NULL);
	}

	// Αλλιώς φτιάχνουμε δύο records ως πάνω και κάτω όρια
	Record record_from = malloc(sizeof(*record_from));
	record_from->date = date_from;
	record_from->id = 0;
	Record record_to = malloc(sizeof(*record_to));
	record_to->date = date_to;
	record_to->id = INT_MAX;

	// Και βρίσκουμε στο set τα records ανάμεσα σε αυτά τα όρια (NULL αν δεν υπάρχουν)
	List list = set_return_from_to(searchset, (date_from != NULL) ? record_from : NULL, (date_to != NULL) ? record_to : NULL);

	free(record_from);
	free(record_to);
	
	return list;
}

// Επιστρέφει τον αριθμό εγγραφών που ικανοποιούν τα συγκεκριμένα κριτήρια.

int dm_count_records(String disease, String country, Date date_from, Date date_to) {
	// Επιλέγουμε το map ανάλογα με το πόσες πληροφορίες έχουμε για την χώρα
	// και μέσα από από αυτό βρίσκουμε το κατάλληλο set
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

	// Αν δεν βρούμε τέτοιο set τότε δεν υπάρχουν κατάλληλα
	// records και επιστρέφουμε 0
	if (searchset == NULL) {
		return 0;
	}

	// Δημιουργούμε ένα record ως κάτω όριο 
	Record date_record = malloc(sizeof(*date_record));
	date_record->date = date_from;
	date_record->id = 0;
	// Μετράμε τα records κάτω από αυτό
	int before = (date_from != NULL) ? set_count_less_than(searchset, date_record) : 0;

	// Δημιουργούμε ένα record ως πάνω όριο 
	date_record->date = date_to;
	date_record->id = INT_MAX;
	// Μετράμε τα records πάνω από αυτό
	int after = (date_to != NULL) ? set_count_greater_than(searchset, date_record) : 0;

	free(date_record);

	// Επιστρέφουμε το πλήθος όλων των εγγραφών μείον αυτών εκτός των ορίων
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

	// Βρίσκουμε την κατάλληλη pqueue ανάλογα με τον ψάχνουμε τις ασθένειες σε μια χώρα ή γενικά
	if (country != NULL) {
		diseases = map_find(country_to_pq, country);
	}
	else {
		diseases = total_pq;
	}

	// Αν δεν υπάρχει τέτοια επιστρέφουμε κενή λίστα
	if (diseases == NULL) {
		return top_diseases;
	}

	// Παίρουμε τις πρώτες k ασθένειες
	top_nodes = pqueue_top_k(diseases, k);

	// ΑΛλά αφού βρίσκονται σε κόμβους μαζί με το πλήθπς των κρουσμάτων,
	// δημιουργούμε μια λίστα που θα περιέχει μόνο τις ασθένειες
	if (list_size(top_nodes)) {
		list_insert_next(top_diseases, LIST_BOF, ((DisCases) list_node_value(top_nodes, list_first(top_nodes)))->disease);
	}
	ListNode node1, node2;
	for (node1 = list_next(top_nodes, list_first(top_nodes)), node2 = list_first(top_diseases);
		node1 != LIST_EOF;
		node1 = list_next(top_nodes, node1), node2 = list_next(top_diseases, node2)) {
			list_insert_next(top_diseases, node2, ((DisCases) list_node_value(top_nodes, node1))->disease);
	}

	// Δεν χρειαζόμαστε την λίστα με τους κόμβους ασθένειας-κρουσμάτων
	list_destroy(top_nodes);

	// Επιστρέφουμε την λίστα με τις ασθένειες
	return top_diseases;
}