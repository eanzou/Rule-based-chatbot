#include <iostream>
#include <fstream>
#include <windows.h>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <bits/stdc++.h>
#include <ctime>
using namespace std;

const string delim = "?!.;,*-_";

class Learner {
public:
    void exactQuestion(string phrase);
    void exactKeyword(string phrase);
    void guessKeyword(string phrase);
    void clean(string &phrase);
    void requestAns(string phrase);
    double calculateSimi(string user, string ai);

private:
    void redundantSpaces(string &phrase);
    bool isPunc(char c);
    void punctuation(string &phrase);
    void lowercase(string &phrase);
    void addSpaces(string &phrase);
    double charSimi(string user, string ai);
    int countWord(string sentence);
    double wordSimi(string user, string ai);
    int max_index(vector<double>&similarity);
    string getKey(string key);
};

main()
{
    Learner AI;

    string phrase;

    cout << "HI! I am a chatbot that can help you find the answer your questions, please key in your question" << endl;

    while (1)
    {
        cout << "User: ";
        getline(cin, phrase);

        if (phrase.size() == 0) //If user did not enter anything, ask the user to enter again
        {
            string response = "You did not ask anything, mind typing in again?";
            cout << "AI: " << response << endl;
        }
        else if (phrase.find("bye") != std::string::npos) //If user type in bye, stop the program
        {
            string response = "Bye, see you next time!";
            cout << "AI: " << response << endl;
            break;
        }
        else
        {
            AI.clean(phrase);
            AI.exactQuestion(phrase); //search for response
        }
    }
    return 0;
}

void Learner::clean(string &phrase)
{
        lowercase(phrase);
        punctuation(phrase);
        redundantSpaces(phrase);
        addSpaces(phrase);
}

void Learner::lowercase(string &phrase)
{
    /*make all letters lowercase*/
    for (int i=0; i<phrase.size(); i++)
        phrase[i]=tolower(phrase[i]);
    return;
}

bool Learner::isPunc(char c)
{
	return delim.find(c) != string::npos;
}

void Learner::punctuation(string &phrase)
{
    /*remove all punctuation from user input*/
    for (int i=0; i<phrase.size(); i++)
    {
        while (isPunc(phrase[i]))
            phrase.erase(phrase.begin()+i);
    }
    return;
}

void Learner::redundantSpaces(string &phrase)
{
    /*remove redundant spaces*/
    for (int i=0; i<phrase.size(); i++)
    {
        while (phrase[i]==' ' && phrase[i-1]==' ')
        phrase.erase(phrase.begin()+i);
    }
    return;
}

void Learner::addSpaces(string &phrase)
{
    /*add spaces at the beginning and at the end of the sentence*/
    phrase.insert(phrase.begin(), ' ');
    phrase.insert(phrase.end(), ' ');
}

void Learner::exactQuestion(string phrase)
{
    fstream dataset;
    dataset.open("Database/dataset.txt", ios::in);    // Open the memory file for input

    // Search through the file until the end is reached
    while( !dataset.eof() ) // While not at end of file
    {
        string identifier;
        getline(dataset,identifier, '\t');    // Get next phrase
        clean(identifier);

        if(identifier == phrase)
        {
            string response;
            getline(dataset,response, '\t');   // If so, get the response
            cout << "AI: " << response << endl << endl;
            dataset.close();
            return;    // Leave the function
        }
    }

    dataset.close();
    exactKeyword(phrase);
    return;
}

void Learner::exactKeyword(string phrase)
{
    fstream dataset;
    vector<double> similarity;
    vector<string> related_questions; //array to keep questions
    string question;
    vector<string> related_answers; //array to keep answers
    string answer;

    dataset.open("Database/dataset.txt", ios::in); //open again to find keyword
    while( !dataset.eof() ) // While not at end of file
    {
        string keyWord;
        getline(dataset,keyWord, '\t');    // Get next phrase

        clean(keyWord);

        if(phrase.find(keyWord) != std::string::npos) //if key word found in the user sentence
        {
            getline(dataset,question, '\t');   // If so, get the question
            related_questions.push_back(question); //put related question in a vector array
            getline(dataset,answer, '\t');   // get the answer of the question
            answer.erase(std::remove(answer.begin(), answer.end(), '\n'), answer.end()); //remove new line from answer
            related_answers.push_back(answer);
            similarity.push_back(calculateSimi(phrase, question)); //put calculated similarity in another vector array
        }
    }

    dataset.close();

    if (related_questions.size() > 0)
    {
        int num_ask_user = 5;

        if (related_questions.size() < num_ask_user)
            num_ask_user = related_questions.size();

        for (int i=0; i<num_ask_user; i++)
        {
            int index = max_index(similarity);
            question = related_questions[index];
            cout << "AI: Is your question '" << question << "'? (enter 'yes' if correct)" << endl;
            string reply;
            cout << "User: ";
            getline(cin, reply);

            if (reply == "yes")
            {
                cout << "AI: " << related_answers[index] << endl << endl;
                return;
            }
        }
        requestAns(phrase); //if no questions in database are related to the user's question, request answer from user
        return;
    }
    guessKeyword(phrase); //if could not find exact keyword, guess keyword
    return;
}

void Learner::guessKeyword(string phrase)
{
    fstream dataset;
    vector<string> key; //array to store keywords
    string ktemp;

    dataset.open("Database/dataset.txt", ios::in); //open again to find keyword
    while( !dataset.eof() ) // While not at end of file
    {
        getline(dataset,ktemp);
        string cur_key = getKey(ktemp);

        bool save = false; //current key unsaved
        for (int i=0; i<key.size(); i++)
        {
            if (cur_key == key[i])
            {
                save = true; //if found same key in array, mark current key as saved
            }
        }

        if (save == false) //if after checking still did not found the key, save it in array
            key.push_back(cur_key);
    }
    dataset.close();

    vector<double> key_simi;
    vector<string> separate; //an array to store separated user sentence
    vector<int> user_index; //an array to keep track of the index to change
    stringstream split(phrase);
    string stemp;

    while(split >> stemp)
        separate.push_back(stemp);

    for (int i=0; i<key.size(); i++)
    {
        if (key[i].size() > 1)
        {
            double max_simi = -1;
            double max_index = -1;

            for (int j=0; j<separate.size(); j++)
            {
                double cur_simi = -1;
                cur_simi = charSimi(separate[j], key[i]); //compare current keyword with all words in user sentence

                if (cur_simi > max_simi)    //only keep the highest similarity
                {
                    max_simi = cur_simi;
                    max_index = j;
                }

            }
            key_simi.push_back(max_simi); //keep the highest similarity in an array
            user_index.push_back(max_index); //keep the index of the suspected word error
        }
    }

    int guessWord = 5;
    for (int i=0; i<guessWord; i++)
    {
        int index = max_index(key_simi);
        string askUser = "Is your question related to " + key[index] + "? (enter 'yes' if correct)";
        cout << "AI: " << askUser << endl;

        string reply;
        cout << "User: ";
        getline(cin, reply);

        if (reply == "yes")
        {
            int index_change = user_index[index];
            separate[index_change] = key[index]; //change user typo

            string edited = separate[0];
            for (int j=1; j<separate.size(); j++)
            {
                edited = edited + " " + separate[j]; //combine the string back together
            }
            clean(edited);
            exactQuestion(edited);
            return;
        }
    }
    requestAns(phrase);
    return;
}

string Learner::getKey(string key)
{
    string first;
    stringstream split(key);

    split >> first;

    return first; //return only the first word which is the key
}

void Learner::requestAns(string phrase)
{
    fstream dataset;

    dataset.open("Database/dataset.txt", ios::out | ios::app);    // Now open for output, and append at end of file

    string userRespond;
    string response = "I don't know the answer, can you help me to update my knowledge base? What is the key word to your question?";
    cout << "AI: " << response << endl;
    cout << "Key Word: ";
    getline(cin, userRespond);    // Get the ideal response
    dataset << "\t" << userRespond << "\t";    // Write the key word to memory
    dataset << phrase << "\t";    // Record initial question in memory

    response = "What is the answer to the question you are asking?";
    cout << "AI: " << response << endl;
    cout << "Answer: ";
    string userRespond2;
    getline(cin, userRespond2);
    dataset << userRespond2 << endl;
    dataset.close();    // Close the file!
    cout << endl;

    return;
}

int Learner::max_index(vector<double> &similarity)
{
    double max = similarity[0];
    int index = 0;

    for(int i=1 ; i<similarity.size() ; i++)
    {
        if(similarity[i] > max)
        {
            max = similarity[i];
            index = i;
        }
    }

    similarity[index] = -1;

    return index;
}

double Learner::calculateSimi(string user, string ai)
{
    clean(ai); //clean response before calculating similarity

    double weight_char;
    double weight_word;
    double char_simi = charSimi(user, ai);
    double word_simi = wordSimi(user, ai);

    if (word_simi > 0.5 || char_simi < 0.5) //assume word matching is more important
    {
        weight_char = 0;
        weight_word = 1;
    }
    else if (char_simi > 0.5 || word_simi < 0.5) //assume character matching is more important
    {
        weight_char = 1;
        weight_word = 0;
    }
    else //if no condition met, give both fair amount of weightage
    {
        weight_char = 0.5;
        weight_word = 0.5;
    }

    double result = (char_simi*weight_char) + (word_simi*weight_word);

    return result;
}

double Learner::charSimi(string user, string ai)
{
    double charMatch = 0;

    for (int i=0; i<user.size(); i++) //check character matching similarity
    {
        if (ai[i] == user[i])
        {
            charMatch++;
        }
    }

    double charPercent = (charMatch/(ai.size()));

    return charPercent;
}

int Learner::countWord(string sentence)
{
    //assuming words are separated by spaces
    //since the string had added spaces in front and behind, it need to be deducted
    return count(sentence.begin(), sentence.end(), ' ') - 1;
}

double Learner::wordSimi(string user, string ai)
{
    double wordMatch = 0;

    int userWord = countWord(user);
    int aiWord = countWord(ai);
    vector<string> separatedUser; //an array to store separated sentence
    stringstream split(user);
    string temp;

    while(split >> temp)
        separatedUser.push_back(temp);

    for (int i=0; i<userWord; i++)
    {
        for (int j=0; j<aiWord; j++)
        {
            if(ai.find(separatedUser[i]) != std::string::npos)
            {
                wordMatch++;
                break;
            }
        }
    }

    double wordPercent = wordMatch/aiWord;

    return wordPercent;
}
