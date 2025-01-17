#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define CONSTQUANTITY 64

// TODO: allocate whole pages to speed up
// TODO: remove global variables

int quantity;
bool * visited;
bool mod_pn, mod_cw;
// mod_pn is true if presence_needed was modified
// mod_cw is true if certain_word was modified

// ----------- CONSTRAINTS --------------

typedef struct {
    // not present: -1, not initialized: 0, present: 1
    int *presence;
    // doesn't belong: -2, not initialized: -1
    int cardinality;
    bool exact_number;
} constraintCell;

void resetConstraints(constraintCell * cArr, int length, bool firstTime) {
    if (firstTime) {
        for (int i = 0; i < CONSTQUANTITY; i++) {
            cArr[i].presence = (int *) malloc (sizeof(int) * length);
            cArr[i].cardinality = -1;
            for (int j = 0; j < length; j++) {
                cArr[i].presence[j] = 0;
            }
            cArr[i].exact_number = false;
        }
    } else {
        for (int i = 0; i < CONSTQUANTITY; i++) {
            cArr[i].cardinality = -1;
            for (int j = 0; j < length; j++) {
                cArr[i].presence[j] = 0;
            }
            cArr[i].exact_number = false;
        }
    }
}

int constraintMapper(char character) {
    int ASCII_value = (int) character;
    if (ASCII_value > 64 && ASCII_value < 91) {
        // Capital Letters (0 - 25)
        ASCII_value = ASCII_value - 65;
    } else if (ASCII_value > 96 && ASCII_value < 123) {
        // Lowercase Letters (26 - 50)
        ASCII_value = ASCII_value - 71;
    } else if (ASCII_value > 47 && ASCII_value < 58) {
        // Numbers (51 - 60)
        ASCII_value = ASCII_value + 4;
    } else if (ASCII_value == 45) {
        // Minus
        ASCII_value = 62;
    } else if (ASCII_value == 95) {
        // Lower bar
        ASCII_value = 63;
    }
    return ASCII_value;
}

// -------------- LIST -----------------

struct nodeLIST {
    char *word;
    struct nodeLIST *next;
};

struct nodeLIST * newNodeList(char *word) {
    struct nodeLIST * new_node;
    new_node = (struct nodeLIST *) malloc (sizeof(struct nodeLIST));
    new_node->word = word;
    new_node->next = NULL;
    return new_node;
}

void printList (struct nodeLIST * node) {
    struct nodeLIST *temp = node;
    while (temp != NULL) {
        printf("%s\n", temp->word);
        temp = temp->next;
    }
}

// TODO: change this below, taken from the internet
void insertNode(struct nodeLIST ** root, struct nodeLIST * temp) {
	if (*root == NULL || strcmp((*root)->word, temp->word) > 0) {
		temp->next = *root;
		*root = temp;
		return;
	}
    struct nodeLIST* current = *root;
	while (current->next != NULL && strcmp(current->next->word, temp->word) < 0)
		current = current->next;

	temp->next = current->next;
	current->next = temp;
}

void resetList(struct nodeLIST ** root) {
    struct nodeLIST * temp = *root, * succ;
    while (temp != NULL) {
        succ = temp->next;
        free(temp);
        temp = succ;
    }
    * root = NULL;
}

// -------------- UTILS ----------------

bool compare(char *ref_word, char *new_word, char *result_word, char *certain_word, char *presence_needed, constraintCell *cArr, int length) {
    bool win_flag = true;
    int constraintValue, tempCardinality;
    memset(visited, false, length);
    for (int i = 0; i < length; i++) {
        if (!visited[i]) {
            constraintValue = constraintMapper(new_word[i]);
            tempCardinality = 0;
            for (int j = i; j < length; j++) {
                if (new_word[i] == new_word[j]) {
                    visited[j] = true;
                    if (new_word[j] == ref_word[j]) {
                        result_word[j] = '+';
                        tempCardinality++;
                        if (certain_word[j] != new_word[j]) {
                            certain_word[j] = new_word[j];
                            mod_cw = true;
                        }
                    } else {
                        win_flag = false;
                        if (strchr(ref_word, new_word[j]) == NULL) {
                            result_word[j] = '/';
                            cArr[constraintValue].cardinality = -2;
                        } else {
                            int d, n, c;
                            d = n = c = 0;
                            for (int t = 0; t < length; t++) {
                                if (ref_word[t] == new_word[i]) {
                                    n++;
                                    if (ref_word[t] == new_word[t]) {
                                        c++;
                                    }
                                }
                            }
                            for (int t = 0; t < j; t++) {
                                if (new_word[t] == new_word[i]) {
                                    if (new_word[t] != ref_word[t]) {
                                        d++;
                                    }
                                }
                            }
                            if (d >= (n-c)) {
                                result_word[j] = '/';
                                cArr[constraintValue].presence[j] = -1;
                                cArr[constraintValue].exact_number = true;
                            } else {
                                result_word[j] = '|';
                                cArr[constraintValue].presence[j] = -1;
                                if (strchr(presence_needed, new_word[i]) == NULL) {
                                    if (strchr(certain_word, new_word[i]) == NULL) {
                                        int z = 0;
                                        while (presence_needed[z] != '*' && z < length) {
                                            z++;
                                        }
                                        presence_needed[z] = new_word[i];
                                        mod_pn = true;
                                    }
                                }
                                tempCardinality++;
                            }
                        }
                    }
                }
            }
            if (cArr[constraintValue].cardinality >= -1 && tempCardinality > cArr[constraintValue].cardinality) {
                cArr[constraintValue].cardinality = tempCardinality;
            }
        }
    }
    return win_flag;
}

bool checkCertainWord(const char * word, const char * cw, int k) {
    for (int i = 0; i < k; i++) {
        if (cw[i] != '*') {
            if (cw[i] != word[i]) {
                return true;
            }
        }
    }
    return false;
}

bool checkPresenceNeeded(const char * word, const char * pn, int k) {
    bool found;
    for (int i = 0; i < k; i++) {
        if (pn[i] != '*') {
            found = false;
            for (int j = 0; j < k && !found; j++) {
                if (word[j] == pn[i]) {
                    found = true;
                }
            }
            if (!found) {
                return true;
            }
        }
    }
    return false;
}

bool heavyCheckBan(constraintCell * constraints, char * temp, char *cw, char *pn, int k) {

    if ((mod_cw && checkCertainWord(temp, cw, k)) || (mod_pn && checkPresenceNeeded(temp, pn, k))) {
        return true;
    }

    int charCount;
    constraintCell tempConstraint;

    for (int i = 0; i < k; i++) {
        visited[i] = false;
    }

    for (int i = 0; i < k; i++) {
        if (!visited[i]) {
            tempConstraint = constraints[constraintMapper(temp[i])];
            if (tempConstraint.cardinality != -1) {
                if (tempConstraint.cardinality == -2) {
                    return true;
                }
                charCount = 0;
                for (int z = i; z < k; z++) {
                    if (temp[z] == temp[i]) {
                        visited[z] = true;
                        if (tempConstraint.presence[z] == -1) {
                            return true;
                        }
                        charCount++;
                        if (tempConstraint.exact_number) {
                            if (tempConstraint.cardinality < charCount) {
                                return true;
                            }
                        }
                    }
                }
                if (tempConstraint.cardinality > charCount) {
                    return true;
                } else {
                    if (tempConstraint.exact_number) {
                        if (tempConstraint.cardinality != charCount) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void banwords(struct nodeLIST ** root, char * cw, char * pn, constraintCell * constraints, int k) {
    // cw: certain_word, pn: presence_needed
    struct nodeLIST *temp = *root, *prev = NULL;
    while (temp != NULL) {
        if (heavyCheckBan(constraints, temp->word, cw, pn, k)) {
            if (*root == temp) {
                *root = (*root)->next;
                free(temp);
                temp = *root;
            } else {
                prev->next = temp->next;
                free(temp);
                temp = prev->next;
            }
            quantity--;
        } else {
            prev = temp;
            temp = temp->next;
        }
    }
}

int getWord(char *temp_word, int length) {
    int i = 0;
    char temp;
    do {
        temp = (char) getchar_unlocked();
        if (temp != EOF) {
            temp_word[i] = temp;
            i++;
        } else {
            return -1;
        }
    } while (temp != '\n');

    if (temp_word[0] == '+') {
        temp = temp_word[12];
        switch (temp) {
            case 't': // +nuova_partita
                return 1;
            case 'r': // +stampa_filtrate
                return 2;
            case 'n': // +inserisci_inizio
                return 3;
            case 'i': // +inserisci_fine
                return 4;
            default:
                return -1;
        }
    }
    temp_word[length] = '\0';
    return 0;
}

// --------------- BST TREE -----------------

struct nodeBST {
    char * word;
    struct nodeBST * left;
    struct nodeBST * right;
};

struct nodeBST * newNodeRB(char *word, int k) {
    struct nodeBST * new_node;
    new_node = (struct nodeBST *) malloc (sizeof (struct nodeBST) + sizeof(char) * k);
    new_node->word = (char *)(new_node + 1);
    strcpy(new_node->word, word);
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

// TODO: change this, taken from the internet
struct nodeBST * insertNodeRB(struct nodeBST * root, char * word, int k, struct nodeBST ** wordPointer) {
    struct nodeBST * newnode = newNodeRB(word, k);
    struct nodeBST * x = root;
    struct nodeBST * y = NULL;

    while (x != NULL) {
        y = x;
        if (strcmp(word, x->word) < 0)
            x = x->left;
        else
            x = x->right;
    }

    if (y == NULL)
        y = newnode;
    else if (strcmp(word, y->word) < 0)
        y->left = newnode;
    else
        y->right = newnode;
    *wordPointer = newnode;
    return y;
}

void newListFiltered(constraintCell * constraints, struct nodeBST *node, struct nodeLIST **root, struct nodeLIST **head, char *cw, char *pn, int k) {
    if (node != NULL) {
        newListFiltered(constraints, node->left, root, head, cw, pn, k);
        if (!heavyCheckBan(constraints, node->word, cw, pn, k)) {
            if (*root == NULL) {
                *root = newNodeList(node->word);
                *head = *root;
            } else {
                (*head)->next = newNodeList(node->word);
                *head = (*head)->next;
            }
            quantity++;
        }
        newListFiltered(constraints, node->right, root, head, cw, pn, k);
    }
}

// TODO: Change this, from the internet
struct nodeBST * searchRB(struct nodeBST * node, char * word) {
    while (node != NULL) {
        int retCode = strcmp(word, node->word);
        if (retCode > 0)
            node = node->right;
        else if (retCode < 0)
            node = node->left;
        else
            return node;
    }
    return NULL;
}

int main() {
    bool winner_flag, filtered_flag, new_insertion_flag;
    int i, k, n, code, rc;
    char *temp_word, *reference_word, *result_word, *certain_word, *presences_needed;
    struct nodeBST* rootRB = NULL;
    struct nodeBST* wordPointer = NULL;
    struct nodeLIST* rootLIST = NULL;
    struct nodeLIST* headLIST = NULL;
    constraintCell constraints[CONSTQUANTITY];

    // acquire length:
    rc = scanf("%d\n", &k);
    rc = rc + 1;

    // acquire words:
    temp_word = (char *) malloc(sizeof(char) * k);
    do {
        code = getWord(temp_word, k);
        if (code == 0) {
            if (rootRB == NULL) {
                rootRB = insertNodeRB(rootRB, temp_word, k, &wordPointer);
            } else {
                insertNodeRB(rootRB, temp_word, k, &wordPointer);
            }
        }
    } while (code == 0);

    resetConstraints(constraints, k, true);

    // define reference word:
    reference_word = (char *) malloc(sizeof(char) * k);
    result_word = (char *) malloc(sizeof(char) * k);
    certain_word = (char *) malloc(sizeof(char) * k);
    presences_needed = (char *) malloc(sizeof(char) * k);
    visited = (bool *) malloc (sizeof(bool) * k);

    new_insertion_flag = false;

    // new game begins
    do {
        quantity = 0;

        getWord(reference_word, k);

        // acquire tries quantity
        rc = scanf("%d\n", &n);
        rc = rc + 1;

        // set certain_word & presences_needed to '*'
        for (int c = 0; c < k; c++) {
            certain_word[c] = '*';
            presences_needed[c] = '*';
        }

        i = 0;
        bool list_generated = false;
        winner_flag = filtered_flag = false;

        do {
            code = getWord(temp_word, k);
            if (code == 0) {
                if (filtered_flag) {
                    insertNodeRB(rootRB, temp_word, k, &wordPointer);
                    if (list_generated) {
                        mod_pn = mod_cw = true;
                        if (!heavyCheckBan(constraints, temp_word, certain_word, presences_needed, k)) {
                            quantity++;
                            struct nodeLIST * tempNode = newNodeList(wordPointer->word);
                            insertNode(&rootLIST, tempNode);
                        }
                        mod_pn = mod_cw = false;
                    }
                } else {
                    if (searchRB(rootRB, temp_word) != NULL) {
                        new_insertion_flag = false;
                        winner_flag = compare(reference_word, temp_word, result_word, certain_word, presences_needed, constraints, k);
                        if (!list_generated) {
                            // if its the first time of a new game, init list by removing words
                            mod_pn = mod_cw = true;
                            newListFiltered(constraints, rootRB, &rootLIST, &headLIST, certain_word, presences_needed, k);
                            list_generated = true;
                            mod_pn = mod_cw = false;
                        } else {
                            // else, simply ban words
                            banwords(&rootLIST, certain_word, presences_needed, constraints, k);
                            mod_pn = mod_cw = false;
                        }
                        if (!winner_flag) {
                            printf("%s\n%d\n", result_word, quantity);
                        } else {
                            printf("ok\n");
                        }
                        i++;
                    } else {
                        printf("not_exists\n");
                    }
                }
            } else if (code == 2) {
                if (!list_generated) {
                    // if its the first time of a new game, init list by removing words
                    mod_pn = mod_cw = true;
                    newListFiltered(constraints, rootRB, &rootLIST, &headLIST, certain_word, presences_needed, k);
                    mod_pn = mod_cw = false;
                    list_generated = true;
                } else {
                    if (new_insertion_flag) {
                        new_insertion_flag = false;
                        banwords(&rootLIST, certain_word, presences_needed, constraints, k);
                        mod_pn = mod_cw = false;
                    }
                }
                printList(rootLIST);
            } else if (code == 3) {
                filtered_flag = true;
            } else if (code == 4) {
                filtered_flag = false;
                new_insertion_flag = true;
            }
        } while (i < n && !winner_flag && code != -1);

        if (!winner_flag && code != -1) {
            printf("ko\n");
        }

        do {
            code = getWord(temp_word, k);
            switch (code) {
                case 0:
                    insertNodeRB(rootRB, temp_word, k, &wordPointer);
                    break;
                case 1:
                    resetList(&rootLIST);
                    resetConstraints(constraints, k, false);
                    break;
                case 2:
                    break;
                case 3:
                    filtered_flag = true;
                    break;
                case 4:
                    filtered_flag = false;
                    new_insertion_flag = true;
                    break;
                default:
                    break;
            }
        } while (code != 1 && code != -1);
    } while (code != -1);
}
