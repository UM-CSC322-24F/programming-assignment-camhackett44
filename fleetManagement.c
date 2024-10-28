#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Constants for maximum boats and name length
#define MAX_BOATS 120
#define MAX_NAME_LENGTH 128

// Enum for different location types
typedef enum {
    SLIP,
    LAND,
    TRAILOR,
    STORAGE
} LocationType;

// Union for holding location information
typedef union {
    int slipNumber;
    char bayLetter;
    char licenseTag[MAX_NAME_LENGTH];
    int storageNumber;
} LocationInfo;

//Struct to represent a Boat
typedef struct {
    char name[MAX_NAME_LENGTH]; //boat's name
    int length; // length of boat in feet
    LocationType locationType; //type of storage - enum
    LocationInfo locationInfo; //where it is stored - union
    double amountOwed; // amount owed to marina
} Boat;

// array of up to 120 pointers to Boat structs 
Boat *boatArray[MAX_BOATS];
int numBoats = 0;


// Function prototypes 
void loadBoatsFromCSV(const char *fileName);
void saveBoatsToCSV(const char *fileName);
void printInventory();
void addBoat();
void removeBoat();
void acceptPayment();
void updateMonthlyFees();
void displayMenu();
LocationType getLocationType(char *locationStr);
int compareBoats(const void *a, const void *b);
void freeMemory();


int main(int argc, char *argv[]) {
    //Check if CSV file name is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <CSV file>\n", argv[0]);
        return 1;
    }

    //Load boat data from the given CSV file 
    loadBoatsFromCSV(argv[1]);

    //welcome message
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");


    //Beginning of menu loop
    int exit = 0;
    while (!exit) {
        //Display menu and force user's input to upper case
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        char choice;
        scanf(" %c", &choice);
        choice = toupper(choice); // make input case-insensitive

        //Menu options
        switch (choice) {
            case 'I':
                printInventory(); // prints sorted inventory
                break;
            case 'A':
                addBoat(); // adds a new boat
                break;
            case 'R':
                removeBoat(); // removes a boat by name
                break;
            case 'P':
                acceptPayment(); // process payment for a boat
                break;
            case 'M':
                updateMonthlyFees(); // updates amount owed for all boats
                break;
            case 'X':
                saveBoatsToCSV(argv[1]); // save data before exiting
                printf("\nExiting the Boat Management System\n");
                exit = 1; // exit loop
                break;
            default:
                printf("Invalid option %c\n", choice); // state if invalid input
        }
    }
    
    freeMemory();
    return 0;
}

// Load boat data from CSV file 
void loadBoatsFromCSV(const char *fileName) {
    FILE *file = fopen(fileName, "r");
    //check for missing file
    if (!file) {
        fprintf(stderr, "Error: Could not open file \n");
        return;
    }



    // Read each line and extract details
    char line[256];
    while (fgets(line, sizeof(line), file) && numBoats < MAX_BOATS) {

        //each boat dynamically allocated with error check
        Boat *newBoat = (Boat *)malloc(sizeof(Boat));
        if (!newBoat) {
            fprintf(stderr, "Memory allocation error.\n");
            fclose(file);
            return;
        }

        // Read data from CSV line into each category
        char locationStr[10], extraInfo[MAX_NAME_LENGTH];
        sscanf(line, "%127[^,],%d,%9[^,],%127[^,],%lf",
               newBoat->name, &newBoat->length, locationStr, extraInfo, &newBoat->amountOwed);

        // Set location type and specific location 
        newBoat->locationType = getLocationType(locationStr);
        switch (newBoat->locationType) {
            case SLIP:
                //(use atoi for string to int maybe)
                newBoat->locationInfo.slipNumber = atoi(extraInfo);
                break;
            case LAND:
                newBoat->locationInfo.bayLetter = extraInfo[0];
                break;
            case TRAILOR:
                strncpy(newBoat->locationInfo.licenseTag, extraInfo, MAX_NAME_LENGTH);
                break;
            case STORAGE:
                newBoat->locationInfo.storageNumber = atoi(extraInfo);
                break;
        }
        //increase boat count after each boat is read
        boatArray[numBoats++] = newBoat;
    }
    fclose(file);
}

// Saves boats to CSV file 
void saveBoatsToCSV(const char *fileName) {
    FILE *file = fopen(fileName, "w");
    //check for missing file
    if (!file) {
        fprintf(stderr, "Error: Could not open file \n");
        return;
    }

    // Write each boat's data to file in CSV format
    for (int i = 0; i < numBoats; i++) {
        fprintf(file, "%s,%d,", boatArray[i]->name, boatArray[i]->length);

        // Write location-specific data
        switch (boatArray[i]->locationType) {
            case SLIP:
                fprintf(file, "slip,%d,", boatArray[i]->locationInfo.slipNumber);
                break;
            case LAND:
                fprintf(file, "land,%c,", boatArray[i]->locationInfo.bayLetter);
                break;
            case TRAILOR:
                fprintf(file, "trailor,%s,", boatArray[i]->locationInfo.licenseTag);
                break;
            case STORAGE:
                fprintf(file, "storage,%d,", boatArray[i]->locationInfo.storageNumber);
                break;
        }

        
        fprintf(file, "%.2f\n", boatArray[i]->amountOwed);
    }
    fclose(file);
}

// Print boat inventory sorted alphabetically 
void printInventory() {

    //"keep the array packed and sorted by boat name"
    qsort(boatArray, numBoats, sizeof(Boat *), compareBoats);
    printf("Fleet Report:\n");

    // Loop through and print each boat's details
    for (int i = 0; i < numBoats; i++) {
        printf("%-21s %2d' ", boatArray[i]->name, boatArray[i]->length);

        // Print location-specific data
        switch (boatArray[i]->locationType) {

            case SLIP:
                printf("   slip   # %2d   ", boatArray[i]->locationInfo.slipNumber);
                break;
            case LAND:
                printf("   land      %c   ", boatArray[i]->locationInfo.bayLetter);
                break;
            case TRAILOR:
                printf("trailor %6s   ", boatArray[i]->locationInfo.licenseTag);
                break;
            case STORAGE:
                printf("storage   # %2d   ", boatArray[i]->locationInfo.storageNumber);
                break;
        }

        printf("Owes $%7.2f\n", boatArray[i]->amountOwed);
    }
}


// Add a new boat 
void addBoat() {

    //check if boat limit has been reached
    if (numBoats >= MAX_BOATS) {
        printf("Marina is full\n");
        return;
    }

    // Dynamically allocate memory for the new boat
    Boat *newBoat = (Boat *)malloc(sizeof(Boat));
    //allocation error check
    if (!newBoat) {
        fprintf(stderr, "Memory allocation error\n");
        return;
    }

    // ask user for boat name
    char locationStr[10], extraInfo[MAX_NAME_LENGTH];
    printf("Please enter the boat data in CSV format                 : ");
    scanf(" %127[^,],%d,%9[^,],%127[^,],%lf",
          newBoat->name, &newBoat->length, locationStr, extraInfo, &newBoat->amountOwed);

    // Set location type and specific info based on location (see line 205)
    newBoat->locationType = getLocationType(locationStr);
    switch (newBoat->locationType) {
        case SLIP:
            newBoat->locationInfo.slipNumber = atoi(extraInfo);
            break;
        case LAND:
            newBoat->locationInfo.bayLetter = extraInfo[0];
            break;
        case TRAILOR:
            strncpy(newBoat->locationInfo.licenseTag, extraInfo, MAX_NAME_LENGTH);
            break;
        case STORAGE:
            newBoat->locationInfo.storageNumber = atoi(extraInfo);
            break;
    }

    // increase count as new boat added
    boatArray[numBoats++] = newBoat;
    //printf("\n");
}

// Remove a boat 
void removeBoat() {
    //ask user for boat name
    char name[MAX_NAME_LENGTH];
    printf("Please enter the boat name                               : ");
    scanf(" %127[^\n]", name);

    //search through array for boat
    int found = 0;
    for (int i = 0; i < numBoats; i++) {
        if (strcasecmp(boatArray[i]->name, name) == 0) {
            //unallocate memory at location of found boat
            free(boatArray[i]);

            // Shift boats down to fill the removed spot
            for (int j = i; j < numBoats - 1; j++) {
                boatArray[j] = boatArray[j + 1];
            }
            numBoats--;
            found = 1;
            
            break;
        }
    }
    //state if boat not found
    if (!found) {
        printf("No boat with that name\n");
    }
}

// Accept payment for a boat
void acceptPayment() {
    //ask user for boat name
    char name[MAX_NAME_LENGTH];
    printf("Please enter the boat name                               : ");
    scanf(" %127[^\n]", name);

    //search through array for boat
    int found = 0;
    for (int i = 0; i < numBoats; i++) {
        if (strcasecmp(boatArray[i]->name, name) == 0) {
            found = 1;

            //get payment amount from user
            double payment;
            printf("Please enter the amount to be paid                       : ");
            scanf("%lf", &payment);


            //handle case where user pays more than is owed
            if (payment > boatArray[i]->amountOwed) {
                printf("That is more than the amount owed, $%.2f \n", boatArray[i]->amountOwed);
            } else {
                boatArray[i]->amountOwed -= payment;
                
            }
            break;
        }
    }
    //state if boat not found
    if (!found) {
        printf("No boat with that name\n");
    }
}

// Update amount owed if a new month has started
void updateMonthlyFees() {
    for (int i = 0; i < numBoats; i++) {

        //iterate over each boat in array, 
        // check location type and updating amount owed based on location
        switch (boatArray[i]->locationType) {
            case SLIP:
                boatArray[i]->amountOwed += 12.5 * boatArray[i]->length;
                break;
            case LAND:
                boatArray[i]->amountOwed += 14.0 * boatArray[i]->length;
                break;
            case TRAILOR:
                boatArray[i]->amountOwed += 25.0 * boatArray[i]->length;
                break;
            case STORAGE:
                boatArray[i]->amountOwed += 11.2 * boatArray[i]->length;
                break;
        }
    }
}


// Function to convert location string to a LocationType enum
LocationType getLocationType(char *locationStr) {
    if (strcasecmp(locationStr, "slip") == 0) return SLIP;
    if (strcasecmp(locationStr, "land") == 0) return LAND;
    if (strcasecmp(locationStr, "trailor") == 0) return TRAILOR;
    if (strcasecmp(locationStr, "storage") == 0) return STORAGE;
    
    // Default to storage if input isnt recognized
    return STORAGE;
}



// Comparison function for qsort
int compareBoats(const void *a, const void *b) {
    Boat *boatA = *(Boat **)a;
    Boat *boatB = *(Boat **)b;
    return strcasecmp(boatA->name, boatB->name);
}

// Free all dynamically allocated memory before exiting to prevent any leaks
void freeMemory() {
    for (int i = 0; i < numBoats; i++) {
        free(boatArray[i]);
    }
}


