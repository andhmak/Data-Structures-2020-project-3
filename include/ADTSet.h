////////////////////////////////////////////////////////////////////////
//
// ADT Set
//
// Abstract διατεταγμένο σύνολο. Τα στοιχεία είναι διατεταγμένα με βάση
// τη συνάρτηση compare, και καθένα εμφανίζεται το πολύ μία φορά.
// Παρέχεται γρήγορη αναζήτηση με ισότητα αλλά και με ανισότητα.
//
////////////////////////////////////////////////////////////////////////

#pragma once // #include το πολύ μία φορά

#include "common_types.h"
#include "ADTList.h"

// Ενα σύνολο αναπαριστάται από τον τύπο Set

typedef struct set* Set;


// Δημιουργεί και επιστρέφει ένα σύνολο, στο οποίο τα στοιχεία συγκρίνονται με βάση
// τη συνάρτηση compare.
// Αν destroy_value != NULL, τότε καλείται destroy_value(value) κάθε φορά που αφαιρείται ένα στοιχείο.

Set set_create(CompareFunc compare, DestroyFunc destroy_value);

// Επιστρέφει τον αριθμό στοιχείων που περιέχει το σύνολο set.

int set_size(Set set);

// Προσθέτει την τιμή value στο σύνολο, αντικαθιστώντας τυχόν προηγούμενη τιμή ισοδύναμη της value.
// Επιστρέφει true αν υπήρχε ισοδύναμη τιμή, αλλιώς false.
//
// ΠΡΟΣΟΧΗ:
// Όσο το value είναι μέλος του set, οποιαδήποτε μεταβολή στο περιεχόμενό του (στη μνήμη που δείχνει) δεν πρέπει
// να αλλάζει τη σχέση διάταξης (compare) με οποιοδήποτε άλλο στοιχείο, διαφορετικά έχει μη ορισμένη συμπεριφορά.

void set_insert(Set set, Pointer value);

// Αφαιρεί τη μοναδική τιμή ισοδύναμη της value από το σύνολο, αν υπάρχει.
// Επιστρέφει true αν βρέθηκε η τιμή αυτή, false διαφορετικά.

bool set_remove(Set set, Pointer value);

// Επιστρέφει την μοναδική τιμή του set που είναι ισοδύναμη με value, ή NULL αν δεν υπάρχει

Pointer set_find(Set set, Pointer value);

// Αλλάζει τη συνάρτηση που καλείται σε κάθε αφαίρεση/αντικατάσταση στοιχείου σε
// destroy_value. Επιστρέφει την προηγούμενη τιμή της συνάρτησης.

DestroyFunc set_set_destroy_value(Set set, DestroyFunc destroy_value);

// Ελευθερώνει όλη τη μνήμη που δεσμεύει το σύνολο.
// Οποιαδήποτε λειτουργία πάνω στο set μετά το destroy είναι μη ορισμένη.

void set_destroy(Set set);


// Διάσχιση του set ////////////////////////////////////////////////////////////
//
// Η διάσχιση γίνεται με τη σειρά διάταξης.

// Οι σταθερές αυτές συμβολίζουν εικονικούς κόμβους _πριν_ τον πρώτο και _μετά_ τον τελευταίο κόμβο του set
#define SET_BOF (SetNode)0
#define SET_EOF (SetNode)0

typedef struct set_node* SetNode;

// Επιστρέφουν τον πρώτο και τον τελευταίο κομβο του set, ή SET_BOF / SET_EOF αντίστοιχα αν το set είναι κενό

SetNode set_first(Set set);
SetNode set_last(Set set);

// Επιστρέφουν τον επόμενο και τον προηγούμενο κομβο του node, ή SET_EOF / SET_BOF
// αντίστοιχα αν ο node δεν έχει επόμενο / προηγούμενο.

SetNode set_next(Set set, SetNode node);
SetNode set_previous(Set set, SetNode node);

// Επιστρέφει το περιεχόμενο του κόμβου node

Pointer set_node_value(Set set, SetNode node);

// Βρίσκει το μοναδικό στοιχείο στο set που να είναι ίσο με value.
// Επιστρέφει τον κόμβο του στοιχείου, ή SET_EOF αν δεν βρεθεί.

SetNode set_find_node(Set set, Pointer value);




//// Επιπλέον συναρτήσεις προς υλοποίηση στο Εργαστήριο 5

// Δείκτης σε συνάρτηση που "επισκέπτεται" ένα στοιχείο value

typedef void (*VisitFunc)(Pointer value);

// Καλεί τη visit(value) για κάθε στοιχείο του set σε διατεταγμένη σειρά

void set_visit(Set set, VisitFunc visit);

// Επιστρέφει μια λίστα με τα στοιχεία από το from μέχρι το to (σύμφωνα με την compare) με πολυπλοκότητα O(logn)
// (σε αυτήν την υλοποίηση) για σταθερό m, με n όλα τα στοιχεία και m αυτά που θα επιστραφούν.
// Αν from ή to είναι NULL δεν τίθεται κάτω ή πάνω όριο, αντίστοιχα.

List set_return_from_to(Set set, Pointer from, Pointer to);

// Μετρούν τα στοιχεία του set μεγαλύτερα από max ή μικρότερα από min, σύμφωνα με την compare, αντίστοιχα.
// Έχουν πολυπλοκότητα O(logn) (σε αυτήν την υλοποίηση) ως προς το μέγεθος του set, ανεξάρτητα από το πλήθος των στοιχείων που μετρούνται.

int set_count_greater_than(Set set, Pointer max);

int set_count_less_than(Set set, Pointer min);