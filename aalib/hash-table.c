#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
 
 
#include "hashtools.h"
 
/** forward declaration */
static HashAlgorithm lookupNamedHashStrategy(const char *name);
static HashProbe lookupNamedProbingStrategy(const char *name);
 
/**
 * Create a hash table of the given size,
 * which will use the given algorithm to create hash values,
 * and the given probing strategy
 *
 *  @param  hash  the HashAlgorithm to use
 *  @param  probingStrategy algorithm used for probing in the case of
 *              collisions
 *  @param  newHashSize  the size of the table (will be rounded up
 *              to the next-nearest larger prime, but see exception)
 *  @see         HashAlgorithm
 *  @see         HashProbe
 *  @see         Primes
 *
 *  @throws java.lang.IndexOutOfBoundsException if no prime number larger
 *              than newHashSize can be found (currently only primes
 *              less than 5000 are known)
 */
AssociativeArray *
aaCreateAssociativeArray(
        size_t size,
        char *probingStrategy,
        char *hashPrimary,
        char *hashSecondary
    )
{
    AssociativeArray *newTable;
 
    newTable = (AssociativeArray *) malloc(sizeof(AssociativeArray));
 
    newTable->hashAlgorithmPrimary = lookupNamedHashStrategy(hashPrimary);
    newTable->hashNamePrimary = strdup(hashPrimary);
    newTable->hashAlgorithmSecondary = lookupNamedHashStrategy(hashSecondary);
    newTable->hashNameSecondary = strdup(hashPrimary);
    newTable->hashProbe = lookupNamedProbingStrategy(probingStrategy);
    newTable->probeName = strdup(probingStrategy);
 
    newTable->size = getLargerPrime(size);
    
    if (newTable->size < 1) {
        fprintf(stderr, "Cannot create table of size %ld\n", size);
        free(newTable);
        return NULL;
    }
 
    newTable->table = (KeyDataPair *) malloc(newTable->size * sizeof(KeyDataPair));
 
    /** initialize everything with zeros */
    memset(newTable->table, 0, newTable->size * sizeof(KeyDataPair));
 
    newTable->nEntries = 0;
 
    newTable->insertCost = newTable->searchCost = newTable->deleteCost = 0;
 
    return newTable;
}
 
/**
 * deallocate all the memory in the store -- the keys (which we allocated),
 * and the store itself.
 * The user * code is responsible for managing the memory for the values
 */
void
aaDeleteAssociativeArray(AssociativeArray *aarray)
{
    /**
     * TO DO:  clean up the memory managed by our utility
     *
     * Note that memory for keys are managed, values are the
     * responsibility of the user
     */
     for (size_t i = 0; i < aarray->size; i++) {
        if (aarray->table[i].validity != HASH_EMPTY) {
            free(aarray->table[i].key);
        }
    }
 
    free(aarray->table);
    free(aarray->hashNamePrimary);
    free(aarray->hashNameSecondary);
    free(aarray->probeName);
    free(aarray);
}
 
/**
 * iterate over the array, calling the user function on each valid value
 */
int aaIterateAction(
        AssociativeArray *aarray,
        int (*userfunction)(AAKeyType key, size_t keylen, void *datavalue, void *userdata),
        void *userdata
    )
{
    int i;
 
    for (i = 0; i < aarray->size; i++) {
        if (aarray->table[i].validity == HASH_USED) {
            if ((*userfunction)(
                    aarray->table[i].key,
                    aarray->table[i].keylen,
                    aarray->table[i].value,
                    userdata) < 0) {
                return -1;
            }
        }
    }
    return 1;
}
 
/** utilities to change names into functions, used in the function above */
static HashAlgorithm lookupNamedHashStrategy(const char *name)
{
    if (strncmp(name, "sum", 3) == 0) {
        return hashBySum;
    } else if (strncmp(name, "len", 3) == 0) {
        return hashByLength;
    }
    else if (strncmp(name, "oddeven", 7) == 0) {
        return hashByOddEven; // New hashing strategy
    }

    fprintf(stderr, "Invalid hash strategy '%s' - using 'sum'\n", name);
    return hashBySum;
}
 
static HashProbe lookupNamedProbingStrategy(const char *name)
{
    if (strncmp(name, "lin", 3) == 0) {
        return linearProbe;
    } else if (strncmp(name, "qua", 3) == 0) {
        return quadraticProbe;
    } else if (strncmp(name, "dou", 3) == 0) {
        return doubleHashProbe;
    }
 
    fprintf(stderr, "Invalid hash probe strategy '%s' - using 'linear'\n", name);
    return linearProbe;
}
 
/**
 * Add another key and data value to the table, provided there is room.
 *
 *  @param  key  a string value used for searching later
 *  @param  value a data value associated with the key
 *  @return      the location the data is placed within the hash table,
 *               or a negative number if no place can be found
 */
int aaInsert(AssociativeArray *aarray, AAKeyType key, size_t keylen, void *value)
{
    /**
     * TO DO:  Search for a location where this key can go, stopping
     * if we find a value that has been delete and reuse it.
     *
     * If a suitable location is found, we then initialize that
     * slot with the new key and data
     */
 
    if (aarray->nEntries >= aarray->size) {
       
        return -1;
    }
 
    HashIndex index = aarray->hashAlgorithmPrimary(key, keylen, aarray->size);
    index = aarray->hashProbe(aarray, key, keylen, index, 1, &aarray->insertCost);
 
    if (index == -1) {
        
        return -1;
    }
 
    aarray->table[index].key = (AAKeyType)strdup((const char *)key);
    aarray->table[index].keylen = keylen;
    aarray->table[index].value = value;
    aarray->table[index].validity = HASH_USED;
    aarray->nEntries++;
 
    return index;
}
 
 
/**
 * Locates the KeyDataPair associated with the given key, if
 * present in the table.
 *
 *  @param  key  the key to search for
 *  @return      the KeyDataPair containing the key, if the key
 *               was present in the table, or NULL, if it was not
 *  @see         KeyDataPair
 */

void *aaLookup(AssociativeArray *aarray, AAKeyType key, size_t keylen) {
    HashIndex index = aarray->hashAlgorithmPrimary(key, keylen, aarray->size);
    int attempt = 0;
 
    while (attempt < aarray->size) {
        HashIndex currentIndex = (index + attempt) % aarray->size;
        KeyDataPair currentPair = aarray->table[currentIndex];
 
       
        aarray->searchCost++;
 
 
        if (currentPair.validity == HASH_USED && keylen == currentPair.keylen && memcmp(key, currentPair.key, keylen) == 0) {
            
            return currentPair.value;
        }
 
        attempt++;
    }
 
    return NULL; 
}
 
/**
 * Locates the KeyDataPair associated with the given key, if
 * present in the table.
 *
 *  @param  key  the key to search for
 *  @return      the index of the KeyDataPair containing the key,
 *               if the key was present in the table, or (-1),
 *               if no key was found
 *  @see         KeyDataPair
 */
void *aaDelete(AssociativeArray *aarray, AAKeyType key, size_t keylen)
{
    /**
     * TO DO: Deletion is closely related to lookup;
     * you must find where the key is stored before
     * you delete it, after all.
     *
     * Implement a deletion algorithm based on tombstones,
     * as described in class
     */
    HashIndex index = aarray->hashAlgorithmPrimary(key, keylen, aarray->size);
    int counter = 0;
 
    while (counter < aarray->size) {
        HashIndex currentIndex = (index + counter) % aarray->size;
        KeyDataPair currentPair = aarray->table[currentIndex];
 
        
        aarray->deleteCost++;
 
        
 
        if (currentPair.validity == HASH_USED && keylen == currentPair.keylen && memcmp(key, currentPair.key, keylen) == 0) {
           
            aarray->table[currentIndex].validity = HASH_DELETED;
            void *value = aarray->table[currentIndex].value;
           
            return value;
        }
 
        counter++;
    }
 
    return NULL; 
}
 
/**
 * Print out the entire aarray contents
 */
void aaPrintContents(FILE *fp, AssociativeArray *aarray, char * tag)
{
    char keybuffer[128];
    int i;
 
    fprintf(fp, "%sDumping aarray of %d entries:\n", tag, aarray->size);
    for (i = 0; i < aarray->size; i++) {
        fprintf(fp, "%s  ", tag);
        if (aarray->table[i].validity == HASH_USED) {
            printableKey(keybuffer, 128,
                    aarray->table[i].key,
                    aarray->table[i].keylen);
            fprintf(fp, "%d : in use : '%s'\n", i, keybuffer);
        } else {
            if (aarray->table[i].validity == HASH_EMPTY) {
                fprintf(fp, "%d : empty (NULL)\n", i);
            } else if ( aarray->table[i].validity == HASH_DELETED) {
                printableKey(keybuffer, 128,
                        aarray->table[i].key,
                        aarray->table[i].keylen);
                fprintf(fp, "%d : empty (deleted - was '%s')\n", i, keybuffer);
            } else {
                fprintf(fp, "%d : invalid validity state %d\n", i,
                        aarray->table[i].validity);
            }
        }
    }
}
 
 
/**
 * Print out a short summary
 */
void aaPrintSummary(FILE *fp, AssociativeArray *aarray)
{
   
    fprintf(fp, "Associative array contains %d entries in a table of %d size\n",
            aarray->nEntries, aarray->size);
    fprintf(fp, "Strategies used: '%s' hash, '%s' secondary hash and '%s' probing\n",
            aarray->hashNamePrimary, aarray->hashNameSecondary, aarray->probeName);
    fprintf(fp, "Costs accrued due to probing:\n");
    fprintf(fp, "  Insertion : %d\n", aarray->insertCost);
    fprintf(fp, "  Search    : %d\n", aarray->searchCost);
    fprintf(fp, "  Deletion  : %d\n", aarray->deleteCost);
}