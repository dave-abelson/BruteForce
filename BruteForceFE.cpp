#include <iostream>
#include <vector>
#include <set>
#include <mutex>
#include <fstream>
#include <iterator>
#include <ctime>

#include <math.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

#define MAX_THREADS 13

struct thread_data {
    string word;
    long length;
    vector<string>* left;
    vector<string>* right;
};

struct status_data {
    vector<string>* left;
    vector<string>* right;
};

pthread_mutex_t left_mutex;
pthread_mutex_t right_mutex;
pthread_mutex_t finished_mutex;
int finished_threads = 0;

int cksum(string password) {
    int a = 0;
    int b = 0;
    for (int i = 0; i < password.length(); i ++) {
        a = (a + password[i]) % 0xff;
        b = (b + a) % 0xff;
    }
    return (b << 8) | a;
}

bool validate_left(string password) {
    return (cksum(password) == 0xd06e);
}

bool validate_right(string password) {
    return (cksum(password) == 0xf00d);
}

char next_value(char character) {
    char newChar = character + 1;
    if (newChar == '[') newChar = 'a';
    if (newChar == '{') newChar = 'A';
    return newChar;
}

void* find_parts(void* threadarg) {
    struct thread_data* data = (struct thread_data*) threadarg;
    string word = data->word;
    long length = data->length;
    vector<string>* left = data->left;
    vector<string>* right = data->right;
    while (length > 0) {
        int cursor = 0;
        bool wrapped = true;
        while (wrapped) {
            if (validate_left(word)) {
                pthread_mutex_lock(&left_mutex);
                left->push_back(word);
                pthread_mutex_unlock(&left_mutex);
            }
            if (validate_right(word)) {
                pthread_mutex_lock(&right_mutex);
                right->push_back(word);
                pthread_mutex_unlock(&right_mutex);
            }
            char character = word[cursor];
            char next_character = next_value(character);
            word[cursor] = next_character;
            if (next_character != 'A') wrapped = false;
            cursor ++;
        }
        length --;
    }
    pthread_mutex_lock(&finished_mutex);
    finished_threads ++;
    pthread_mutex_unlock(&finished_mutex);
    pthread_exit(NULL);
}

void* update_status(void* threadarg) {
    struct status_data* data = (struct status_data*) threadarg;
    vector<string>* left = data->left;
    vector<string>* right = data->right;
    while (finished_threads < MAX_THREADS) {
        cout << "Left: " << to_string(left->size()) << ", Right: " << to_string(right->size()) << endl;
        sleep(10);
    }
    pthread_exit(NULL);
}

int main() {

	cout << "Brute forcing with " << to_string(MAX_THREADS) << " threads." << endl;

	time_t start_time = time(nullptr);

	cout << "Starting time: " << asctime(localtime(&start_time));
    
    vector<string> left, right;
    
    pthread_t threads[MAX_THREADS];
    pthread_attr_t attr;
    void* status;
    struct thread_data td[MAX_THREADS];
    int rc;
    
    pthread_mutex_init(&left_mutex, NULL);
    pthread_mutex_init(&right_mutex, NULL);
    pthread_mutex_init(&finished_mutex, NULL);
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	char starting_char = 'A';
	int chars_per_thread = (int) ceil(52.0/MAX_THREADS);
    
    for (int i = 0; i < MAX_THREADS; i ++) {
        td[i].word = "AAAAA";
		td[i].word.push_back(starting_char);
		for (int j = 0; j < chars_per_thread; j ++) starting_char = next_value(starting_char);
        td[i].length = chars_per_thread * pow(52, 5);
        td[i].left = &left;
        td[i].right = &right;
        rc = pthread_create(&threads[i], NULL, find_parts, (void*)&td[i]);
        if (rc) {
            exit(-1);
        }
    }
    
    pthread_t status_thread;
    struct status_data sd;
    sd.left = &left;
    sd.right = &right;
    rc = pthread_create(&status_thread, NULL, update_status, (void*)&sd);
    if (rc) {
        exit(-1);
    }
    
    pthread_attr_destroy(&attr);
    
    for (int i = 0; i < MAX_THREADS; i ++) {
        rc = pthread_join(threads[i], &status);
        if (rc) {
            exit(-1);
        }
    }
    
    pthread_join(status_thread, &status);
    
    cout << "Left (" << to_string(left.size()) << ") and right (" << to_string(right.size()) << ") found." << endl;

	time_t found_time = time(nullptr);

	cout << "Found in " << to_string(found_time - start_time) << " seconds." << endl;

	ofstream left_file("./left");
	ostream_iterator<string> left_iterator(left_file, "\n");
	copy(left.begin(), left.end(), left_iterator);

	ofstream right_file("./right");
	ostream_iterator<string> right_iterator(right_file, "\n");
	copy(right.begin(), right.end(), right_iterator);

	vector<string> left_filtered, right_filtered;
	set<int> left_found, right_found;

	for (string l : left) {
		string even = "";
		for (int i = 0; i < l.length(); i ++) {
			if (i % 2 == 0) even += l[i];
		}
		int ck = cksum(even);
		if (left_found.find(ck) == left_found.end()) {
			left_found.insert(ck);
			left_filtered.push_back(l);
		}
	}

	for (string r : right) {
		string even = "";
		for (int i = 0; i < r.length(); i ++) {
			if (i % 2 == 0) even += r[i];
		}
		int ck = cksum(even);
		if (right_found.find(ck) == right_found.end()) {
			right_found.insert(ck);
			right_filtered.push_back(r);
		}
	}

    for (string l : left_filtered) {
        for (string r : right_filtered) {
			string word = l + r;
			string even = "";
			for (int i = 0; i < word.length(); i ++) {
				if (i % 2 == 0) even += word[i];
			}
            if (cksum(even) == 0x0000) {
                cout << "Password found: " << l << r << endl;
            }
        }
    }
    cout << "All possibilities scanned." << endl;

	time_t end_time = time(nullptr);

	cout << "Finished in " << to_string(end_time - start_time) << " seconds: " << asctime(localtime(&end_time));
    
    pthread_exit(NULL);
    
    return 0;
}
