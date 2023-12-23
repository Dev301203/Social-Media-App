#include "friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * Create a new user with the given name.  Insert it at the tail of the list
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user by this name already exists in this list.
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator).
 */
int create_user(const char *name, User **user_ptr_add) {
    if (strlen(name) >= MAX_NAME) {
        return 2;
    }

    User *new_user = malloc(sizeof(User));
    if (new_user == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_user->name, name, MAX_NAME); // name has max length MAX_NAME - 1

    for (int i = 0; i < MAX_NAME; i++) {
        new_user->profile_pic[i] = '\0';
    }

    new_user->first_post = NULL;
    new_user->next = NULL;
    for (int i = 0; i < MAX_FRIENDS; i++) {
        new_user->friends[i] = NULL;
    }

    // Add user to list
    User *prev = NULL;
    User *curr = *user_ptr_add;
    while (curr != NULL && strcmp(curr->name, name) != 0) {
        prev = curr;
        curr = curr->next;
    }

    if (*user_ptr_add == NULL) {
        *user_ptr_add = new_user;
        return 0;
    } else if (curr != NULL) {
        free(new_user);
        return 1;
    } else {
        prev->next = new_user;
        return 0;
    }
}


/*
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 *
 * NOTE: You'll likely need to cast a (const User *) to a (User *)
 * to satisfy the prototype without warnings.
 */
User *find_user(const char *name, const User *head) {
    while (head != NULL && strcmp(name, head->name) != 0) {
        head = head->next;
    }

    return (User *)head;
}


/*
 * Return a pointer to a dynamically allocated string containing
 * the usernames of all users in the list starting at curr.
 */
char *list_users(const User *curr) {
    if (curr == NULL) {
        // If there is no user in the list, return empty dynamically allocated string
		char *empty = malloc(0);

        // Checking if malloc worked as intended
        if (empty == NULL) {
            perror("malloc");
            exit(1);
        }
        return empty;
    }

    // Used for finding the size of string to be dynamically allocated
    int malloc_size = 0;

    // Used for iterating over every user
    const User *copy_curr = curr;

    // Iterating over the linked list of Users starting at copy_curr
    while (copy_curr != NULL) {

        // Adding length of current name
        malloc_size += strlen(copy_curr->name);

        // Adding space for \r\n
        malloc_size += 2;
        copy_curr = copy_curr->next;
    }

    // Adding space for \0 at the end
    malloc_size += 1;

    // Allocating memory on the heap for string of all usernames
    char* usernames = malloc(malloc_size * sizeof(char));

    // Checking if malloc worked as intended
    if (!usernames) {
        perror("malloc");
        exit(1);
    }

    int written_len = 0;

    // Iterating over the linked list of Users starting at curr
    while (curr != NULL) {
        // Concatenating the curr User's name followed by \r\n
        written_len += snprintf(usernames + written_len, malloc_size - written_len, "%s\r\n", curr->name);
        curr = curr->next;
    }

    // Concatenating the string NULL terminator
    usernames[malloc_size-1] = '\0';
    /* strcat(usernames, "\0"); */

    // Return pointer to string containing all User's names
    return usernames;
}


/*
 * Make two users friends with each other.  This is symmetric - a pointer to
 * each user must be stored in the 'friends' array of the other.
 *
 * New friends must be added in the first empty spot in the 'friends' array.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the two users are already friends.
 *   - 2 if the users are not already friends, but at least one already has
 *     MAX_FRIENDS friends.
 *   - 3 if the same user is passed in twice.
 *   - 4 if at least one user does not exist.
 *
 * Do not modify either user if the result is a failure.
 * NOTE: If multiple errors apply, return the *largest* error code that applies.
 */
int make_friends(const char *name1, const char *name2, User *head) {
    User *user1 = find_user(name1, head);
    User *user2 = find_user(name2, head);

    if (user1 == NULL || user2 == NULL) {
        return 4;
    } else if (user1 == user2) { // Same user
        return 3;
    }

    int i, j;
    for (i = 0; i < MAX_FRIENDS; i++) {
        if (user1->friends[i] == NULL) { // Empty spot
            break;
        } else if (user1->friends[i] == user2) { // Already friends.
            return 1;
        }
    }

    for (j = 0; j < MAX_FRIENDS; j++) {
        if (user2->friends[j] == NULL) { // Empty spot
            break;
        }
    }

    if (i == MAX_FRIENDS || j == MAX_FRIENDS) { // Too many friends.
        return 2;
    }

    user1->friends[i] = user2;
    user2->friends[j] = user1;
    return 0;
}


/*
 *  Return the number of characters used while printing a post.
 */
int num_chars_in_printing_post(const Post *post) {
    // Used to store the number of characters used while printing a post
    int number_chars = 0;

    // Since there is no post, no print occurs
    if (post == NULL) {
        return number_chars;
    }
    
    // Characters used in "From: \r\n" : 8 characters
    number_chars += 8;

    // Characters used in authors name
    number_chars += strlen(post->author);

    // Characters used in "Date: " : 6 characters
    number_chars += 6;

    // Characters used in post time. Adding one since it already ends with a \n.
    number_chars += strlen(asctime(localtime(post->date))) + 1;

    // Characters used in "\r\n" : 2 characters
    number_chars += 2;

    // Characters used in post content including \r\n at the end
    number_chars += strlen(post->contents) + 2;

    // Returning number of characters used
    return number_chars;
}


/*
 * Return a pointer to a dynamically allocated string containing
 * a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 */
char *print_user(const User *user) {
    if (user == NULL) {
        // If there is no user in the list, return empty dynamically allocated string
		char *empty = malloc(0);

        // Checking if malloc worked as intended
        if (empty == NULL) {
            perror("malloc");
            exit(1);
        }
        return empty;
    }

    char *dash = "------------------------------------------\r\n";

    // Used for finding the size of string to be dynamically allocated
    int malloc_size = 0;

    // Characters used in User's name
    malloc_size += strlen(user->name);

    // Characters used in "Name: \r\n\r\n" : 10 characters
    malloc_size += 10;

    // Characters used in "Friends:\r\n" : 10 characters
    malloc_size += 10;

    // Characters used in "Posts:\r\n" : 8 characters
    malloc_size += 8;

    // Characters used in 3 lines of dashes
    malloc_size += strlen(dash) * 3;

    // Add length of each friend User's name
    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {

        // Characters used in friend User's name followed by \r\n
        malloc_size += strlen(user->friends[i]->name) + 2;
    }

    // Used for iterating over every post of the User to add space needed
    const Post *curr = user->first_post;
    while (curr != NULL) {

        // Adding space for each post
        malloc_size += num_chars_in_printing_post(curr);
        curr = curr->next;
        if (curr != NULL) {

            // Characters used in "===\r\n" : 5 characters
            malloc_size += 5;
        }
    }

    // Adding space for \0 at the end
    malloc_size += 1;

    // Allocating memory on the heap for string of User profile
    char* user_profile = malloc(malloc_size * sizeof(char));

    // Checking if malloc worked as intended
    if (!user_profile) {
        perror("malloc");
        exit(1);
    }

    int written_len = 0;

    // Write User name to allocated string
    written_len += snprintf(user_profile + written_len, malloc_size - written_len, "Name: %s\r\n\r\n", user->name);

    // Write dashes to allocated string
    written_len += snprintf(user_profile + written_len, malloc_size - written_len, "%s", dash);

    // Write friend User's names to allocated string
    written_len += snprintf(user_profile + written_len, malloc_size - written_len, "Friends:\r\n");
    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
        written_len += snprintf(user_profile + written_len, malloc_size - written_len, "%s\r\n", user->friends[i]->name);
    }

    // Write dashes to allocated string
    written_len += snprintf(user_profile + written_len, malloc_size - written_len, "%s", dash);

    // Going back to first post
    curr = user->first_post;

    // Write User's posts to allocated string
    written_len += snprintf(user_profile + written_len, malloc_size - written_len, "Posts:\r\n");
    while (curr != NULL) {
        // Write post author to allocated string
        written_len += snprintf(user_profile + written_len, malloc_size - written_len, "From: %s\r\n", curr->author);
    
        // Write post time to allocated string
        char *time = asctime(localtime(curr->date));

        // Since asctime returns a string ending with \n, updating it to have \r\n at the end
        time[strlen(time)-1] = '\r';
        written_len += snprintf(user_profile + written_len, malloc_size - written_len, "Date: %s\n\r\n", time);

        // Write post content to allocated string
        written_len += snprintf(user_profile + written_len, malloc_size - written_len, "%s\r\n", curr->contents);
        curr = curr->next;
        if (curr != NULL) {
            written_len += snprintf(user_profile + written_len, malloc_size - written_len, "===\r\n");
        }
    }

    // Write dashes to allocated string
    written_len += snprintf(user_profile + written_len, malloc_size - written_len, "%s", dash);

    // Return pointer to string containing User's profile
    return user_profile;
}


/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * Use the 'time' function to store the current time.
 *
 * 'contents' is a pointer to heap-allocated memory - you do not need
 * to allocate more memory to store the contents of the post.
 *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents) {
    if (target == NULL || author == NULL) {
        return 2;
    }

    int friends = 0;
    for (int i = 0; i < MAX_FRIENDS && target->friends[i] != NULL; i++) {
        if (strcmp(target->friends[i]->name, author->name) == 0) {
            friends = 1;
            break;
        }
    }

    if (friends == 0) {
        return 1;
    }

    // Create post
    Post *new_post = malloc(sizeof(Post));
    if (new_post == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_post->author, author->name, MAX_NAME);
    new_post->contents = contents;
    new_post->date = malloc(sizeof(time_t));
    if (new_post->date == NULL) {
        perror("malloc");
        exit(1);
    }
    time(new_post->date);
    new_post->next = target->first_post;
    target->first_post = new_post;

    return 0;
}

