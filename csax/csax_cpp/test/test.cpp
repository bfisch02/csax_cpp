#include <map>
#include <vector>
#include <iostream>
using namespace std;

struct Person {
    string name;
    vector<string> fav_words;
};

void test(vector<Person*> people);

int main()
{
    vector<Person*> people;
    map<string, Person> people_map;
    Person *p = new Person;
    p->name = "Brett";
    people.push_back(p);
    p->name = "Steve";
    p->fav_words.push_back("Hi");
    test(people);
    Person *l = people[0];
    cout << "Name: " << l->name << endl;
    cout << "Fav word one: " << l->fav_words[1] << endl;
}

void test(vector<Person*> people)
{
    Person *l = people[0];
    l->fav_words.push_back("hello");
}
