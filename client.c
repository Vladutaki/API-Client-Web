#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "parson.h"
#include "requests.h"
#include <stdbool.h>

// Variabile globale pentru caile de acces catre server
#define HOST "34.246.184.49"
#define PORT 8080
#define REGISTER "/api/v1/tema/auth/register"
#define LOGIN "/api/v1/tema/auth/login"
#define ACCESS "/api/v1/tema/library/access"
#define BOOKS "/api/v1/tema/library/books"
#define LOGOUT "/api/v1/tema/auth/logout"
#define TYPE_REQUEST "application/json"

// Variabile globale pentru socket, starea de logare si starea de stergere
int sockfd = -1, logged = 0, delete = 0;

int main(int argc, char *argv[])
{
    char *mesaj, *raspuns, *token, cookie[8000], comanda[21];
    while(1){
        // Initializare obiect JSON
        JSON_Value *valoare = json_value_init_object();
        JSON_Object *obiect = json_value_get_object(valoare);

        // Citire comanda
        memset(comanda, 0, 21);
        fgets(comanda, 20, stdin);
        comanda[strcspn(comanda, "\n")] = '\0';

        // Verificare comanda - Register
        if(strcmp(comanda, "register") == 0){
            char username[51], parola[51];
            // Citire username
            printf("%s", "username=");
            memset(username, 0, 51);
            fgets(username, sizeof(username), stdin);

            // Eliminare caracter newline
            for(int i = 0; i < strlen(username); i++) {
                if(username[i] == '\n') {
                    username[i] = '\0';
                    continue;
                }
            }

            // Verificare username fara spatii
            int username_ok = 1;
            for (int i = 0; username[i] != '\0'; i++) {
                if (username[i] == ' ') {
                    username_ok = 0;
                    break;
                }
            }

            // Citire parola
            printf("%s", "password=");
            memset(parola, 0, 51);
            fgets(parola, sizeof(parola), stdin);

            // Eliminare caracter newline
            for(int i = 0; i < strlen(parola); i++) {
                if(parola[i] == '\n') {
                    parola[i] = '\0';
                    continue;
                }
            }

            // Verificare parola fara spatii
            int i=0;
            int parola_ok = 1;
            while (parola[i] != '\0') {
                if (parola[i] == ' ') {
                    parola_ok = 0;
                    break;
                }
                i++;
            }
            
            // Verificare username si parola
            if(username_ok == 0) {
                printf("ERROR: Username-ul are spatii.\n");
                continue;
            }

            if(parola_ok == 0) {
                printf("ERROR: Parola are spatii.\n");
                continue;
            }

            if (username[0] == '\0') {
                printf("ERROR: Username-ul nu poate fi gol!\n");
                continue;
            }

            if (parola[0] == '\0') {
                printf("ERROR: Parola nu poate fi goala!\n");
                continue;
            }

            // Creare string din obiect JSON
            json_object_set_string(obiect, "username", username);
            json_object_set_string(obiect, "password", parola);
            // Pregatire mesaj pentru server
            char *buffer = json_serialize_to_string_pretty(valoare);

            mesaj = compute_post_request(HOST, REGISTER, TYPE_REQUEST, &buffer, 1, NULL, NULL);

            // Trimitere mesaj catre server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, mesaj);

            // Primire raspuns de la server
            raspuns = receive_from_server(sockfd);
            json_free_serialized_string(buffer);
            json_value_free(valoare);
            close(sockfd);

            // Tratare cazuri de erori
            int tip_raspuns = 0;
            if (strstr(raspuns, " taken") != NULL) {
                tip_raspuns = 1;
            }

            switch (tip_raspuns) {
                case 1:
                    printf("ERROR: Username-ul exista deja.\n");
                    break;
                default:
                    printf("Utilizatorul a fost inregistrat cu succes\n");
                    break;
            }
        }
        // Verificare comanda - Login
        else if(strcmp(comanda, "login") == 0){
            // Pregatire mesaj pentru server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            JSON_Value *valoare = json_value_init_object();
            JSON_Object *obiect = json_value_get_object(valoare);

            // Citire username
            char username[51], parola[51];
            printf("%s", "username= ");
            memset(username, 0, 51);
            fgets(username, sizeof(username), stdin);

            // Eliminare caracter newline
            for(int i = 0; i < strlen(username); i++) {
                if(username[i] == '\n') {
                    username[i] = '\0';
                    continue;
                }
            }

            // Verificare username fara spatii
            int i = 0;
            int username_ok = 1;
            while (username[i] != '\0') {
                if (username[i] == ' ') {
                    username_ok = 0;
                    break;
                }
                i++;
            }

            // Citire parola
            printf("%s", "password= ");
            memset(parola, 0, 51);
            fgets(parola, sizeof(parola), stdin);
            
            // Eliminare caracter newline
            for(int i = 0; i < strlen(parola); i++) {
                if(parola[i] == '\n') {
                    parola[i] = '\0';
                    continue;
                }
            }

            // Verificare parola fara spatii
            int parola_ok = 1;
            while (parola[i] != '\0') {
                if (parola[i] == ' ') {
                    parola_ok = 0;
                    break;
                }
                i++;
            }

            // Verificare username si parola
            if(username_ok == 0) {
                printf("ERROR: Username-ul are spatii.\n");
                continue;
            }

            if(parola_ok == 0) {
                printf("ERROR: Parola are spatii.\n");
                continue;
            }

            if (username[0] == '\0') {
                printf("ERROR: Username-ul nu poate fi gol!\n");
                continue;
            }

            if (parola[0] == '\0') {
                printf("ERROR: Parola nu poate fi goala!\n");
                continue;
            }

            // Creare string din obiect JSON
            json_object_set_string(obiect, "username", username);
            json_object_set_string(obiect, "password", parola);
            char *buffer = json_serialize_to_string_pretty(valoare);

            // Pregatire mesaj pentru server
            mesaj = compute_post_request(HOST, LOGIN, TYPE_REQUEST, &buffer, 1, NULL, NULL);
            json_free_serialized_string(buffer);
            json_value_free(valoare);
            send_to_server(sockfd, mesaj);

            // Primire raspuns de la server
            raspuns = receive_from_server(sockfd);
            
            // Tratare cazuri de erori
            int tip_raspuns = 0;
            if (strstr(raspuns, "Credentials are not good") != NULL) {
                tip_raspuns = 1;
            } else if (strstr(raspuns, "No account with this username") != NULL) {
                tip_raspuns = 2;
            }

            switch (tip_raspuns) {
                case 1:
                    printf("ERROR: Credentiale gresite.\n");
                    break;
                case 2:
                    printf("ERROR: Contul nu exista. \n");
                    break;
                default:
                    printf("Utilizatorul a fost logat cu succes\n");
                    break;
            }

            // Salvare cookie
            char *start = strstr(raspuns, "connect.sid");
            if (start != NULL) {
                char *end = strstr(start, ";");
                size_t cookie_len = end - start;
                strncpy(cookie, start, cookie_len);
                cookie[cookie_len] = '\0'; 
            }
            
            logged = 1;

            close(sockfd);
        }
        // Verificare comanda - Enter_library
        else if(strcmp(comanda, "enter_library") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            if(token != NULL && strlen(token) != 0){
                printf("ERROR: Esti deja in biblioteca. \n");
                continue;
            }
            switch (logged) {
            case 1:
                // Pregatire mesaj pentru server
                mesaj = compute_get_request(HOST, ACCESS, "", cookie, NULL, delete);
                send_to_server(sockfd, mesaj);
                // Primire raspuns de la server
                raspuns = receive_from_server(sockfd);

                // Salvare token
                token = strstr(raspuns, "token");
                if (token) {
                    token += 8;
                    token[strlen(token) - 2] = '\0';
                }

                // Tratare cazuri de erori
                if (raspuns == NULL || strlen(raspuns) == 0) {
                    printf("ERROR: Accesare librarie invalida.\n");
                } else {
                    printf("SUCCES: Utilizatorul are acces la biblioteca\n");
                }
                close(sockfd);
                break;
            case 0:
                printf("ERROR: Nu esti logat! \n");
                continue;
            }
        }
        // Verificare comanda - Get_books
        else if(strcmp(comanda, "get_books") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            if(logged){
                // Pregatire mesaj pentru server
                mesaj = compute_get_request(HOST, BOOKS, "", cookie, token, delete);
                send_to_server(sockfd, mesaj);
                // Primire raspuns de la server
                raspuns = receive_from_server(sockfd);
                
                // Afisare raspuns de la server cu pretty
                JSON_Value *valoare = json_parse_string(strstr(raspuns, "["));
                char *buffer = json_serialize_to_string_pretty(valoare);
                printf("%s\n", buffer);

                close(sockfd);
            }
            else {
                // Tratare cazuri de erori
                printf("ERROR: Nu esti logat!\n");
                continue;
            }
        }
        // Verificare comanda - Get_book
        else if(strcmp(comanda, "get_book") == 0){
            if(logged){
                // Citire id carte
                printf("%s", "id=");
                int id;
                int res = scanf("%d", &id);
                if(res != 1 || getchar() != '\n') {
                    printf("ERROR: ID-ul trebuie sa fie numar valid.\n");
                    continue;
                }
                
                // Pregatire mesaj pentru server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                char addr[60];
                sprintf(addr, "%s/%d", BOOKS, id);

                mesaj = compute_get_request(HOST, addr, "", cookie, token, delete);
                send_to_server(sockfd, mesaj);
                // Primire raspuns de la server
                raspuns = receive_from_server(sockfd);

                // Tratare cazuri de erori
                int tip_eroare = 0;
                if(strstr(raspuns, "No book was found")) {
                    tip_eroare = 1;
                }

                switch (tip_eroare) {
                    case 1:
                        printf("ERROR: Cartea cu id=%d nu exista!\n", id);
                        break;
                    default:
                        JSON_Value *valoare = json_parse_string(strstr(raspuns, "{"));
                        char *buffer = json_serialize_to_string_pretty(valoare);
                        printf("%s\n", buffer);
                        break;
                }

                close(sockfd);
            }
            else {
                // Tratare cazuri de erori
                printf("ERROR: Nu esti logat!\n");
                close(sockfd);
                continue;
            }
        }
        // Verificare comanda - Add_book
        else if(strcmp(comanda, "add_book") == 0){
            // Pregatire mesaj pentru server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            JSON_Value *valoare = json_value_init_object();
			JSON_Object *obiect = json_value_get_object(valoare);

            if(logged && token != NULL){
                // Citire titlu carte
                char titlu[100];
                printf("%s", "title=");
                fgets(titlu, sizeof(titlu), stdin);

                for(int i = 0; i < strlen(titlu); i++) {
                    if(titlu[i] == '\n') {
                        titlu[i] = '\0';
                        continue;
                    }
                }
                json_object_set_string(obiect, "title", titlu);

                // Citire autor carte
                char autor[100];
                printf("%s", "author=");
                fgets(autor, sizeof(autor), stdin);
                for(int i = 0; i < strlen(autor); i++) {
                    if(autor[i] == '\n') {
                        autor[i] = '\0';
                        continue;
                    }
                }
				json_object_set_string(obiect, "author", autor);
                
                // Citire gen carte
                char gen[100];
                printf("%s", "genre=");
                fgets(gen, sizeof(gen), stdin);
                for(int i = 0; i < strlen(gen); i++) {
                    if(gen[i] == '\n') {
                        gen[i] = '\0';
                        continue;
                    }
                }
				json_object_set_string(obiect, "genre", gen);

                // Citire editura carte
                char editor[100];
                printf("%s", "publisher=");
                fgets(editor, sizeof(editor), stdin);
                for(int i = 0; i < strlen(editor); i++) {
                    if(editor[i] == '\n') {
                        editor[i] = '\0';
                        continue;
                }
                }
                json_object_set_string(obiect, "publisher", editor);

                // Citire numar pagini carte
                int nr_pag = 0;
                char nr_pag_str[10];
                printf("%s", "page_count=");
                int res = scanf("%d", &nr_pag);

                if(res != 1){
                    printf("ERROR: Tip de date incorect pentru numarul de pagini\n");
                    continue;
                }

                sprintf(nr_pag_str, "%d", nr_pag);
                json_object_set_string(obiect, "page_count", nr_pag_str);

                // Pregatire mesaj pentru server
                char *buffer = json_serialize_to_string_pretty(valoare);
                
                mesaj = compute_post_request(HOST, BOOKS, TYPE_REQUEST, &buffer, 1, cookie, token);
                send_to_server(sockfd, mesaj);
                // Primire raspuns de la server
                raspuns = receive_from_server(sockfd);
                
                // Tratare cazuri de erori
                int tip_eroare = 0;
                if(strstr(raspuns, "error")) {
                    tip_eroare = 1;
                }
                switch (tip_eroare) {
                    case 1:
                        printf("ERROR: Cartea nu a fost adaugata.\n");
                        break;
                    default:
                        printf("Cartea a fost adaugata cu succes!\n");
                        break;
                }

                json_free_serialized_string(buffer);
                json_value_free(valoare);
                close(sockfd);
            }
            else {
                // Tratare cazuri de erori
                printf("ERROR: Nu esti logat!\n");
                continue;
            }
        }
        // Verificare comanda - Delete_book
        else if(strcmp(comanda, "delete_book") == 0){
            if(logged && token != NULL){
                // Pregatire mesaj pentru server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                delete = true;

                // Citire id carte
                printf("%s", "id=");
                int id;
                if (scanf("%d", &id) != 1 || getchar() != '\n') {
                    printf("ERROR: ID-ul invalid.\n");
                    while (getchar() != '\n');
                    continue;
                }

                char addr[60];
                sprintf(addr, "%s/%d", BOOKS, id);

                // Pregatire mesaj pentru server
                mesaj = compute_get_request(HOST, addr, "", cookie, token, delete);
                send_to_server(sockfd, mesaj);
                // Primire raspuns de la server
                raspuns = receive_from_server(sockfd);
                // Tratare cazuri de erori
                int tip_eroare = 0;
                if(strstr(raspuns, "No book was found")) {
                    tip_eroare = 1;
                }

                switch (tip_eroare) {
                    case 1:
                        printf("ERROR: ID invalid. Cartea nu a fost gasita.\n");
                        break;
                    default:
                        printf("Cartea cu id %d a fost stearsa cu succes!\n", id);
                        break;
                }

                delete = false;
                close(sockfd);
            }
            else {
                // Tratare cazuri de erori
                printf("ERROR: Nu esti logat! \n");
                continue;
            }
        }
        // Verificare comanda - Logout
        else if(strcmp(comanda, "logout") == 0){
            if(logged){
                // Pregatire mesaj pentru server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                mesaj = compute_get_request(HOST, LOGOUT, "", cookie, token, delete);
                send_to_server(sockfd, mesaj);
                // Primire raspuns de la server
                raspuns = receive_from_server(sockfd);

                printf("Utilizatorul s-a delogat cu succes!\n");

                // Resetare variabile
                logged = 0;
                memset(cookie, 0, 8000);
                token = "";
                close(sockfd);
            }
            else {
                // Tratare cazuri de erori
                printf("ERROR: Nu esti conectat.\n");
                continue;
            }
        }
        // Verificare comanda - Exit
        else if(strcmp(comanda, "exit") == 0){
            // Terminam programul
            break;
        }
    }
    return 0;
}
