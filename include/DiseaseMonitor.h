///////////////////////////////////////////////////////////////////
//
// Disease Monitor
//
// Module που παρέχει στατιστικά για την εξέλιξη μιας ασθένειας.
//
///////////////////////////////////////////////////////////////////

#pragma once // #include το πολύ μία φορά

#include "ADTList.h"

// Οι ημερομηνίες δίνονται σαν Strings, σε format YYYY-MM-DD, πχ "2019-10-31"
typedef String Date;

// O monitor αποθηκεύει εγγραφές του παρακάτω τύπου
// _Ολα_ τα πεδία πρέπει να _υπάρχουν_ και να _μην είναι NULL_

struct record {
	int id;				// Μοναδικό id της εγγραφής
	String name;		// Όνομά
	String disease;		// Ασθένεια
	String country;		// Χώρα
	Date date;			// Ημερομηνία, σε μορφή YYYY-MM-DD
};
typedef struct record* Record;


// Αρχικοποιεί όλες τις δομές του monitor, πρέπει να κληθεί πριν από οποιαδήποτε άλλη κλήση.
// Αν υπήρχαν ήδη δεδομένα τα διαγράφει καλώντας την dm_destroy.

void dm_init();

// Καταστρέφει όλες τις δομές του monitor, απελευθερώνοντας την αντίστοιχη
// μνήμη. ΔΕΝ κάνει free τα records, αυτά δημιουργούνται και καταστρέφονται από
// τον χρήστη.

void dm_destroy();


// Προσθέτει την εγγραφή record στο monitor. Δεν δεσμεύει νέα μνήμη (ούτε
// φτιάχνει αντίγραφα του record), απλά αποθηκεύει τον pointer (η δέσμευση
// μνήμης για τα records είναι ευθύνη του χρήστη). Αν υπάρχει εγγραφή με το ίδιο
// id αντικαθίσταται και επιστρέφεται true, αν όχι επιστρέφεται false.
//
// Οι αλλαγές στα δεδομένα της εγγραφής απαγορεύονται μέχρι να γίνει remove από
// τον monitor.

bool dm_insert_record(Record record);

// Αφαιρεί την εγγραφή με το συγκεκριμένο id από το σύστημα (χωρίς free, είναι
// ευθύνη του χρήστη). Επιστρέφει true αν υπήρχε τέτοια εγγραφή, αλλιώς false.

bool dm_remove_record(int id);


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

List dm_get_records(String disease, String country, Date date_from, Date date_to);

// Επιστρέφει τον αριθμό εγγραφών που ικανοποιούν τα συγκεκριμένα κριτήρια.

int dm_count_records(String disease, String country, Date date_from, Date date_to);

// Επιστρέφει τις k ασθένειες με τις περισσότερες εγγραφές που ικανοποιούν τo
// κριτήριο country (μπορεί να είναι NULL) _ταξινομημένες_ με βάση τον αριθμό
// εγγραφών (πρώτα η ασθένεια με τις περισσότερες).
//
// Πχ η dm_top_diseases(3, "Germany") επιστρέφει τις 3 ασθένειες με τις
// περισσότερες εγγραφές στη Γερμανία. Επιστρέφονται _μόνο_ ασθένειες με
// __τουλάχιστον 1 εγγραφή__ (που ικανοποιεί τα κριτήρια).

List dm_top_diseases(int k, String country);

