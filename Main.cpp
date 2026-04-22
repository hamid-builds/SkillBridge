#include "Message.h"
int main()
{
    cout << "========================================" << endl;
    cout << "       SkillBridge  -  Messaging" << endl;
    cout << "========================================" << endl;
    Message msgSystem;
    int choice;
    do {
        cout << "\n1. Send Message\n";
        cout << "2. View My Messages\n";
        cout << "0. Exit\n";
        cout << "========================================" << endl;
        cout << "  Choice: ";
        cin >> choice;
        cin.ignore();
        if (choice == 1){
            int sID, rID;
            string msg;
            cout << "Your User ID  : "; cin >> sID;
            cout << "Receiver ID   : "; cin >> rID;
            cin.ignore();
            cout << "Message       : "; getline(cin, msg);
            msgSystem.sendMessage(sID, rID, msg);
        }
        else if (choice == 2) {
            int uID;
            cout << "Your User ID  : "; cin >> uID;
            int count = 0;
            Message* res = msgSystem.viewMessages(uID, count);
            if (res) {
                cout << "\n" << count << " message(s) for User " << uID << ":\n";
                for (int i = 0; i < count; i++)
                    res[i].display();
                cout << "  ----------------------------------------\n";
            }
        }
        else if (choice != 0)
        {
            cout << "[Error] Invalid choice.\n";
        }
    } while (choice != 0);
    cout << "Goodbye!" << endl;
    return 0;
}