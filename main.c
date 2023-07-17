#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Election
{
    char year[5];
    char branch[6];
    int max_voters;
    int max_candidates;
    FILE **candidates;
    char votes[15][20];
};

void clear_screen()
{
    printf("\033[2J\033[H"); // ANSI escape code to clear screen and move cursor to (0, 0)
}

void pause_program()
{
    printf("Press Enter to continue...");
    while (getchar() != '\n');
}

int admin_login()
{
    FILE *admin_file;
    admin_file = fopen("admin_login.txt", "r");
    char line[10];
    char user[10];
    char password[10];
    int count = 0;

    while (fgets(line, sizeof(line), admin_file))
    {
        if (count == 0)
        {
            line[strcspn(line, "\n")] = '\0'; // Remove the newline character
            strcpy(user, line);
        }

        if (count == 1)
        {
            line[strcspn(line, "\n")] = '\0'; // Remove the newline character
            strcpy(password, line);
        }

        count++;
    }

    fclose(admin_file);

    char user_input[10];
    char password_input[10];
    int count2 = 0;

    while (strcmp(user_input, user) != 0 || strcmp(password_input, password) != 0)
    {
        printf("Enter username\n");
        scanf("%s", user_input);
        printf("Enter password\n");
        scanf("%s", password_input);
        count2++;
        clear_screen();

        if (count2 == 3)
        {
            printf("Too many failed attempts\n");
            pause_program();
            clear_screen();
            return 0;
        }
    }

    return 1;
}

void new_election(struct Election *e1)
{
    printf("Enter year\n");
    scanf("%s", e1->year);
    printf("Enter branch\n");
    scanf("%s", e1->branch);
    printf("Enter maximum number of voters\n");
    scanf("%d", &e1->max_voters);
    printf("Enter maximum candidates\n");
    scanf("%d", &e1->max_candidates);

    e1->candidates = malloc(e1->max_candidates * sizeof(FILE *));
    if (e1->candidates == NULL)
    {
        printf("Memory allocation failed for candidates array\n");
        return;
    }

    for (int i = 0; i < e1->max_candidates; i++)
    {
        char filename[20];
        sprintf(filename, "candidate_%d.txt", i);
        e1->candidates[i] = fopen(filename, "w");
        if (e1->candidates[i] == NULL)
        {
            printf("Failed to create candidate file: %s\n", filename);
            return;
        }
        fprintf(e1->candidates[i], "Candidate %d", i);
        fclose(e1->candidates[i]);
    }
}

void delete_vote(struct Election *e1)
{
    char user_id[15];
    printf("Enter the user ID whose vote you want to delete: ");
    scanf("%s", user_id);

    FILE *voted_users_file = fopen("voted_users.txt", "r");
    if (voted_users_file == NULL)
    {
        printf("Failed to open the voted users file.\n");
        return;
    }

    char line[20];
    char voted_user[20];
    int found = 0;

    FILE *temp_voted_users_file = fopen("temp_voted_users.txt", "w");
    if (temp_voted_users_file == NULL)
    {
        printf("Failed to create temporary voted users file.\n");
        fclose(voted_users_file);
        return;
    }

    while (fgets(line, sizeof(line), voted_users_file))
    {
        strcpy(voted_user, line);
        voted_user[strcspn(voted_user, "\n")] = '\0'; // Remove the newline character

        if (strcmp(user_id, voted_user) == 0)
        {
            found = 1;
        }
        else
        {
            fprintf(temp_voted_users_file, "%s\n", voted_user);
        }
    }

    fclose(voted_users_file);
    fclose(temp_voted_users_file);

    if (found == 0)
    {
        printf("User ID not found or vote already deleted.\n");
        remove("temp_voted_users.txt");
        return;
    }

    if (remove("voted_users.txt") != 0)
    {
        printf("Failed to delete voted_users.txt\n");
        return;
    }
    if (rename("temp_voted_users.txt", "voted_users.txt") != 0)
    {
        printf("Failed to rename temp_voted_users.txt\n");
        return;
    }

    int candidate_number = -1;

    for (int i = 0; i < e1->max_voters; i++)
    {
        if (strncmp(e1->votes[i], user_id, strlen(user_id)) == 0)
        {
        //    printf("WE IN");
        //  candidate_number = (int)e1->votes[i][14];
            candidate_number = e1->votes[i][14] - '0';
            break;
        }

        // else{
        //     printf("UserID: %s \n", user_id);
        //     printf("Local User: %s", e1->votes[i]);
        // }
    }


    if (candidate_number < 0 || candidate_number >= e1->max_candidates)
    {
        printf("Invalid candidate number.\n");
        printf("%d", candidate_number);
        return;
    }

    char filename[20];
    sprintf(filename, "candidate_%d.txt", candidate_number);
    FILE *candidate_file = fopen(filename, "r");
    if (candidate_file == NULL)
    {
        printf("Failed to open the candidate file.\n");
        return;
    }

    FILE *temp_candidate_file = fopen("temp_candidate.txt", "w");
    if (temp_candidate_file == NULL)
    {
        printf("Failed to create temporary candidate file.\n");
        fclose(candidate_file);
        return;
    }

    int vote_deleted = 0;
    while (fgets(line, sizeof(line), candidate_file))
    {
        if (strcmp(line, "Vote") == 0 || strcmp(line, "\nVote") == 0 || strcmp(line, "Vote\n") == 0)
        {
            if (!vote_deleted)
            {
                vote_deleted = 1;
                continue; // Skip writing this vote to the temporary file
            }
        }

        fprintf(temp_candidate_file, "%s", line);
    }


    fclose(candidate_file);
    fclose(temp_candidate_file);

    if (remove(filename) != 0)
    {
        printf("Failed to delete candidate file: %s\n", filename);
        return;
    }
    if (rename("temp_candidate.txt", filename) != 0)
    {
        printf("Failed to rename temp_candidate.txt\n");
        return;
    }

    printf("Vote deleted successfully!\n");

    
}

void ban_user()
{
    char user_id[15];
    printf("Enter the user ID you want to ban: ");
    scanf("%s", user_id);

    FILE *user_file = fopen("user_login.txt", "r");
    if (user_file == NULL)
    {
        printf("Failed to open the user login file.\n");
        return;
    }

    char line[20];
    char user[20];
    int found = 0;

    FILE *temp_file = fopen("temp_user_login.txt", "w");
    if (temp_file == NULL)
    {
        printf("Failed to create temporary file.\n");
        fclose(user_file);
        return;
    }

    while (fgets(line, sizeof(line), user_file))
    {
        strcpy(user, line);
        user[strcspn(user, "\n")] = '\0'; // Remove the newline character

        if (strcmp(user_id, user) == 0)
        {
            found = 1;
            continue; // Skip writing this user to the temporary file
        }

        fprintf(temp_file, "%s\n", user);
    }

    fclose(user_file);
    fclose(temp_file);

    if (found == 0)
    {
        printf("User ID not found or already banned.\n");
        remove("temp_user_login.txt");
        return;
    }

    if (remove("user_login.txt") != 0)
    {
        printf("Failed to delete user_login.txt\n");
        return;
    }
    if (rename("temp_user_login.txt", "user_login.txt") != 0)
    {
        printf("Failed to rename temp_user_login.txt\n");
        return;
    }

    printf("User banned successfully!\n");
}

void count_votes(struct Election *e1)
{
    int count[e1->max_candidates];
    for (int i = 0; i < e1->max_candidates; i++)
    {
        char filename[20];
        sprintf(filename, "candidate_%d.txt", i);
        FILE *candidate_file = fopen(filename, "r");

        if (candidate_file == NULL)
        {
            printf("Failed to open candidate file: %s\n", filename);
            return;
        }

        int vote_count = 0;
        char line[20];

        while (fgets(line, sizeof(line), candidate_file))
        {
            if (strcmp(line, "Vote") == 0 || strcmp(line, "\nVote") == 0 || strcmp(line, "Vote\n") == 0)
            {
                vote_count++;
            }
        }

        fclose(candidate_file);

        printf("Candidate %d: %d votes\n", i, vote_count);
        count[i] = vote_count;
    }

    int max_votes = 0;
    int winner_index = -1;

    for (int i = 0; i < e1->max_candidates; i++)
    {
        if (count[i] > max_votes)
        {
            max_votes = count[i];
            winner_index = i;
        }
    }

    if (winner_index != -1)
    {
        int tie = 0;
        for (int i = 0; i < e1->max_candidates; i++)
        {
            if (count[i] == max_votes && i != winner_index)
            {
                tie = 1;
                break;
            }
        }

        if (tie)
        {
            printf("Tie! There are multiple winners with %d votes:\n", max_votes);
            for (int i = 0; i < e1->max_candidates; i++)
            {
                if (count[i] == max_votes)
                {
                    printf("Candidate %d\n", i);
                }
            }
        }
        else
        {
            printf("Winner of Elections is Candidate %d with %d votes.\n", winner_index, count[winner_index]);
        }
    }
    else
    {
        printf("No winner. All candidates have 0 votes.\n");
    }
}

void admin_menu(struct Election *e1)
{
    if (admin_login() == 0)
        return;

    int x;

    while (x != -1)
    {
        printf("What would you like to do:\n1.Conduct New Election\n2.Delete Illegal Vote\n3.Ban User ID\n4.View Election Results\n5.Logout\n");
        scanf("%d", &x);
        clear_screen();

        switch (x)
        {
            case 1:
                new_election(e1);
                break;
            case 2:
                delete_vote(e1);
                break;
            case 3:
                ban_user();
                break;
            case 4:
                count_votes(e1);
                break;
            case 5:
                {
                    printf("Logging out\n");
                    pause_program();
                    clear_screen();
                    x = -1;
                }
            default:
                x = -1;
                break;
        }
    }
}

int user_login(struct Election *e1)
{
    FILE *user_file;
    user_file = fopen("user_login.txt", "r");
    if (user_file == NULL)
    {
        printf("Failed to open the user login file.\n");
        return 0;
    }

    char line[20];
    char user[20];

    char user_input[20];
    int flag = 0;

    printf("Enter username: ");
    scanf("%s", user_input);
    clear_screen();

    while (fgets(line, sizeof(line), user_file))
    {
        strcpy(user, line);
        user[strcspn(user, "\n")] = '\0'; // Remove the newline character

        if (strcmp(user_input, user) == 0)
        {
            fclose(user_file);
            flag = 1;
            for (int i=0; i<e1->max_voters; i++)
            {
                if (e1->votes[i][0] == '\0')
                {
                //    printf("\nIF IF\n");
                    strcpy(e1->votes[i], user_input);
                //    printf("%s", e1->votes[i]);
                    break;
                }

                // else
                // {
                //     printf("\nELSE ELSE\n");
                // }
            }
            break;
        }
    }

    fclose(user_file);

    if (flag == 0)
    {
        printf("Enter correct user ID\n");
        return 0;
    }

    int year_match = strncmp(user_input, e1->year, 4);
    int branch_match = strncmp(user_input + 4, e1->branch, 5);

    if (year_match == 0 && branch_match == 0)
    {
        FILE *voted_users_file = fopen("voted_users.txt", "r");
        if (voted_users_file == NULL)
        {
            printf("Failed to open the voted users file.\n");
            return 0;
        }

        char voted_user[100][20];
        int voted_user_count = 0;

        while (fgets(line, sizeof(line), voted_users_file))
        {
            strcpy(voted_user[voted_user_count], line);
            voted_user[voted_user_count][strcspn(voted_user[voted_user_count], "\n")] = '\0'; // Remove the newline character
            voted_user_count++;
        }

        fclose(voted_users_file);

        int has_voted = 0;
        for (int i = 0; i < voted_user_count; i++)
        {
            if (strcmp(voted_user[i], user_input) == 0)
            {
                has_voted = 1;
                break;
            }
        }

        if (has_voted)
        {
            printf("You have already cast your vote.\n");
            return 0;
        }

        voted_users_file = fopen("voted_users.txt", "a");
        if (voted_users_file == NULL)
        {
            printf("Failed to open the voted users file.\n");
            return 0;
        }

        fprintf(voted_users_file, "%s\n", user_input);
        fclose(voted_users_file);

        return 1;
    }

    return 0;
}

int cast_vote(struct Election* e1)
{
    // int voted_before = 0;
    // if (voted_before >= 1)
    // {
    //     printf("You can only vote once\n");
    //     return 0;
    // }

    int candidate_number;
    printf("Enter the candidate number you want to vote for: ");
    scanf("%d", &candidate_number);
    char cdnum[20];

    for (int i = 0; i < e1->max_voters; i++)
    {
        if (e1->votes[i][14] == '\0')
        {
            // printf("\n INSIDE \n");
            //sprintf(e1->votes[i][13], " %d", candidate_number);
            sprintf(e1->votes[i], "%s %d", e1->votes[i], candidate_number);
            // printf("%s", e1->votes[i]);
           // strcpy(e1->votes[i], cdnum);
            break;
        }
        // else
        // {   
        //     printf("Coming in else block\n");
        //     printf("\n %s \n", e1->votes[i] );
        // }
    }

    if (candidate_number >= 0 && candidate_number < e1->max_candidates)
    {
        char filename[20];
        sprintf(filename, "candidate_%d.txt", candidate_number);
        FILE *candidate_file = fopen(filename, "a"); // Open the candidate's file in append mode
        if (candidate_file)
        {
            fprintf(candidate_file, "\nVote"); // Write the vote to the candidate's file
            fclose(candidate_file);
            printf("Vote casted successfully!\n");
            return 1;
        }
        else
        {
            printf("Failed to open the candidate's file.\n");
        }
    }
    else
    {
        printf("Invalid candidate number.\n");
    }

    return 0;
}

void user_menu(struct Election *e1)
{
    if (user_login(e1) == 0)
        return;

    int x;
    int already_voted = 0;
    static int count = 0;

    while (x != -1)
    {
        printf("What would you like to do:\n1.Cast Vote\n2.Logout\n");
        scanf("%d", &x);
        clear_screen();

        if (x == 1 && count >= e1->max_voters)
        {
            printf("Maximum numbers of candidates have voted\n");
            continue;
        }

        if (x == 1 && already_voted == 0)
        {
                already_voted = cast_vote(e1);
                count += already_voted;
        }

        else if (x == 1 && already_voted >= 1)
            printf("Already voted\n");

        if (x >= 2)
        {
            printf("Logging out\n");
            pause_program();
            clear_screen();
            x = -1;
        }
    }
}

int main()
{
    int x;
    struct Election e1;
    e1.candidates = NULL;
    // for (int i=0; i<15; i++)
    // {
    //     for (int j=0; j<20; j++)
    //     {
    //         e1.votes[i][j] = '\0';
    //     }
    // }

   // e1.votes[15][20] = {{0}};
    memset(e1.votes, 0, sizeof(e1.votes));

    while (x != -1)
    {
        printf("Would you like to login as\n1.Admin\n2.User\n");
        scanf("%d", &x);
        clear_screen();

        switch (x)
        {
            case 1:
                admin_menu(&e1);
                break;
            case 2:
                user_menu(&e1);
                break;
            default:
                x = -1;
                break;
        }
    }

    free(e1.candidates); // Free the dynamically allocated candidates array

    return 0;
}
