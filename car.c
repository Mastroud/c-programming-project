#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

typedef struct {
    int slot;
    char vehicleNo[20];
    char ticketID[20];
    char type[10];
    int isVIP;
    time_t entryTime;
    int occupied;
} Parking;

double totalRevenue = 0;

int isAlreadyParked(Parking *p, int n, char *num) {
    for (int i = 0; i < n; i++)
        if (p[i].occupied && strcmp(p[i].vehicleNo, num) == 0)
            return 1;
    return 0;
}

void generateTicketID(char *id, int slot) {
    sprintf(id, "TKT%04d%02d", rand() % 9000 + 1000, slot);
}

void logToFile(Parking *v, double sec, double bill) {
    FILE *f = fopen("parking_log.txt", "a");
    if (!f) {
        printf("Error: Unable to open parking_log.txt for writing!\n");
        return;
    }

    char entry[30], exitT[30];
    struct tm *en = localtime(&v->entryTime);
    time_t ex = time(NULL);
    struct tm *exx = localtime(&ex);

    strftime(entry, 30, "%Y-%m-%d %H:%M:%S", en);
    strftime(exitT, 30, "%Y-%m-%d %H:%M:%S", exx);

    fprintf(f,
            "Ticket: %s\nVehicle: %s\nType: %s\nSlot: %d\nVIP: %d\nEntry: "
            "%s\nExit: %s\nTime: %.2f sec\nBill: %.2f\n\n",
            v->ticketID, v->vehicleNo, v->type, v->slot, v->isVIP, entry, exitT,
            sec, bill);

    fclose(f);
}

double getRate(char *type) {
    if (strcmp(type, "bike") == 0)
        return 2;
    if (strcmp(type, "car") == 0)
        return 3;
    if (strcmp(type, "truck") == 0)
        return 5;
    return 2;
}

int isValidType(char *type) {
    return strcmp(type, "bike") == 0 || strcmp(type, "car") == 0 || strcmp(type, "truck") == 0;
}

void showParkingStatus(Parking *p, int n, double revenue) {
    int parkedCount = 0;
    int vipCount = 0;
    int emptyCount = 0;

    for (int i = 0; i < n; i++) {
        if (p[i].occupied) {
            parkedCount++;
            if (p[i].isVIP)
                vipCount++;
        } else {
            emptyCount++;
        }
    }

    printf("\n========== PARKING STATUS ==========\n");
    printf("Total Slots: %d\n", n);
    printf("Occupied Slots: %d\n", parkedCount);
    printf("VIP Slots Occupied: %d\n", vipCount);
    printf("Available Slots: %d\n", emptyCount);
    printf("Total Revenue: Rs %.2f\n", revenue);
    printf("====================================\n");
}

void showParkingMap(Parking *p, int n) {
    int columns = 3;
    int rows = (n + columns - 1) / columns;

    printf("\n============= PARKING MAP =============\n\n");

    for (int r = 0; r < rows; r++) {
        printf("    ");
        for (int c = 0; c < columns; c++) {
            int idx = r * columns + c;
            if (idx < n)
                printf("+----------+   ");
            else
                printf("            ");
        }
        printf("\n    ");
        for (int c = 0; c < columns; c++) {
            int idx = r * columns + c;
            if (idx < n)
                printf("| Slot %-3d |   ", idx + 1);
            else
                printf("            ");
        }
        printf("\n    ");
        for (int c = 0; c < columns; c++) {
            int idx = r * columns + c;
            if (idx < n)
                printf("| %-8s |   ", p[idx].occupied ? p[idx].vehicleNo : "EMPTY");
            else
                printf("            ");
        }
        printf("\n    ");
        for (int c = 0; c < columns; c++) {
            int idx = r * columns + c;
            if (idx < n)
                printf("+----------+   ");
            else
                printf("            ");
        }
        printf("\n\n");
    }
    printf("============== END MAP ==============\n");
}

void parkVehicle(Parking *p, int n) {
    char num[20], type[10];
    int vip;

    printf("Enter Vehicle Number: ");
    scanf("%19s", num);

    if (isAlreadyParked(p, n, num)) {
        printf("Vehicle already parked.\n");
        return;
    }

    printf("Vehicle Type (bike/car/truck): ");
    scanf("%9s", type);

    if (!isValidType(type)) {
        printf("Invalid vehicle type! Only bike, car, truck allowed.\n");
        return;
    }

    printf("VIP Slot? (1=Yes, 0=No): ");
    scanf("%d", &vip);

    for (int i = 0; i < n; i++) {
        if (!p[i].occupied) {
            strcpy(p[i].vehicleNo, num);
            strcpy(p[i].type, type);
            p[i].isVIP = vip;
            p[i].entryTime = time(NULL);
            p[i].occupied = 1;
            generateTicketID(p[i].ticketID, i + 1);

            printf("Vehicle Parked at Slot %d\n", i + 1);
            printf("Ticket ID: %s\n", p[i].ticketID);
            return;
        }
    }

    printf("Parking Full\n");
}

void removeVehicle(Parking *p, int n) {
    char ticket[20], coupon[20];

    printf("Enter Ticket ID: ");
    scanf("%19s", ticket);

    for (int i = 0; i < n; i++) {
        if (p[i].occupied && strcmp(p[i].ticketID, ticket) == 0) {
            time_t exitTime = time(NULL);
            double sec = difftime(exitTime, p[i].entryTime);
            double units = sec / 10.0;
            if (units < 1)
                units = 1;

            double rate = getRate(p[i].type);
            if (p[i].isVIP)
                rate *= 2;

            double bill = units * rate;

            if (sec < 30)
                printf("Stay Category: Short Stay\n");
            else if (sec <= 120)
                printf("Stay Category: Medium Stay\n");
            else
                printf("Stay Category: Long Stay (10%% discount)\n"), bill *= 0.9;

            printf("Apply Coupon? (type NONE for no): ");
            scanf("%19s", coupon);
            // Case-insensitive coupon check
            for (int j = 0; coupon[j]; j++) 
                coupon[j] = toupper(coupon[j]);
            if (strcmp(coupon, "SAVE10") == 0)
                bill *= 0.90;

            totalRevenue += bill;

            printf("Slot: %d\n", i + 1);
            printf("Time Parked: %.2f sec\n", sec);
            printf("Bill: Rs %.2f\n", bill);
            printf("Total Earnings: Rs %.2f\n", totalRevenue);

            logToFile(&p[i], sec, bill);

            p[i].occupied = 0;
            p[i].vehicleNo[0] = '\0';
            p[i].ticketID[0] = '\0';
            return;
        }
    }
    printf("Invalid Ticket ID\n");
}

int main() {
    srand(time(NULL));

    int n, choice;
    printf("Enter number of parking slots: ");
    if (scanf("%d", &n) != 1 || n < 1) {
        printf("Invalid slot number!\n");
        return 1;
    }

    Parking *p = (Parking *)malloc(n * sizeof(Parking));
    for (int i = 0; i < n; i++) {
        p[i].slot = i + 1;
        p[i].occupied = 0;
    }

    while (1) {
        printf("\n1. Park Vehicle\n2. Remove Vehicle\n3. Display Status\n4. Parking Map\n5. Exit\nEnter Choice: ");
        scanf("%d", &choice);

        if (choice == 1)
            parkVehicle(p, n);
        else if (choice == 2)
            removeVehicle(p, n);
        else if (choice == 3)
            showParkingStatus(p, n, totalRevenue);
        else if (choice == 4)
            showParkingMap(p, n);
        else if (choice == 5)
            break;
        else
            printf("Invalid Choice\n");
    }

    free(p);
    return 0;
}
