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

int compare_record_dates(Pointer a, Pointer b) {
	int result = strcmp(((Record) a)->date, ((Record) b)->date);
	if (result) {
		return result;
	}
	return ((Record) a)->id - ((Record) b)->id;
}

int compare_records_dis_country(Pointer a, Pointer b) {
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

uint hash_id(Pointer value) {
	return ((Record) value)->id;
}

static Map dm_map, id_map, dis_map, country_map;
static Set total_set;
// Αρχικοποιεί όλες τις δομές του monitor, πρέπει να κληθεί πριν από οποιαδήποτε άλλη κλήση.
// Αν υπήρχαν ήδη δεδομένα τα διαγράφει καλώντας την dm_destroy.

void dm_init() {
	dm_map = map_create(compare_records_dis_country, NULL, (DestroyFunc) set_destroy);
	map_set_hash_function(dm_map, hash_dis_country);
	dis_map = map_create(compare_diseases, NULL, (DestroyFunc) set_destroy);
	map_set_hash_function(dis_map, hash_disease);
	country_map = map_create(compare_countries, NULL, (DestroyFunc) set_destroy);
	map_set_hash_function(country_map, hash_country);
	id_map =  map_create(compare_ids, NULL, NULL);
	map_set_hash_function(id_map, hash_id);
	total_set = set_create(compare_record_dates, NULL);
}

// Καταστρέφει όλες τις δομές του monitor, απελευθερώνοντας την αντίστοιχη
// μνήμη. ΔΕΝ κάνει free τα records, αυτά δημιουργούνται και καταστρέφονται από
// τον χρήστη.

void dm_destroy() {
	map_destroy(dm_map);
	map_destroy(id_map);
	map_destroy(dis_map);
	map_destroy(country_map);
	set_destroy(total_set);
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
	bool a;
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
	if ((dest_set = map_find(dm_map, record))) {
		return set_insert(dest_set, record);
	}
	else {
		dest_set = set_create(compare_record_dates, NULL);
		map_insert(dm_map, record, dest_set);
		a = set_insert(dest_set, record);
		if (a) {
			exit(1);
		}
		return a;
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
	Set set = map_find(dm_map, record);
	if (set_remove(set, record) == 0) {
		exit(2);
	}
	if (set_size(set) == 0) {
		map_remove(dm_map, record);
	}
	set = map_find(dis_map, record);
	if (set_remove(set, record) == 0) {
		exit(3);
	}
	if (set_size(set) == 0) {
		map_remove(dis_map, record);
	}
	set = map_find(country_map, record);
	if (set_remove(set, record) == 0) {
		exit(4);
	}
	if (set_size(set) == 0) {
		map_remove(country_map, record);
	}
	if (set_remove(total_set, record) == 0) {
		exit(9);
	}
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
	Map searchmap = dm_map;
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
	Map searchmap = dm_map;
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
	Record date_record = malloc(sizeof(*date_record));
	date_record->date = date_from;
	date_record->id = 0;
	int before = (date_from != NULL) ? set_count_less_than(searchset, date_record) : 0;
	date_record->date = date_to;
	date_record->id = INT_MAX;
	int after = (date_to != NULL) ? set_count_greater_than(searchset, date_record) : 0;
	free(date_record);
	if (set_size(searchset) - before - after < 0) {
		exit(6);
	}
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
	return NULL;
}