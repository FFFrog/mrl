#include <iostream>
#include <vector>

using namespace std;

struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(NULL)
    {
    }
};

class Solution {
public:
    ListNode *reverseList(ListNode *head)
    {
        if (head == NULL)
            return NULL;

        ListNode *cur = head;
        ListNode *pre = NULL;
        ListNode *next;

        while (cur->next) {
            next = cur->next;
            cur->next = pre;
            pre = cur;
            cur = next;
        }
        cur->next = pre;

        return cur;
    }

    ListNode* createList(vector<int>& vec)
    {
        if (vec.empty())
            return NULL;

        ListNode* root = new ListNode(vec.at(0));
        ListNode* cur = root;

        for (size_t i = 1; i < vec.size(); i++)
        {
            cur->next = new ListNode(vec.at(i));
            cur = cur->next;
        }

        return root;
    }
};

int main()
{
    Solution sol;

    vector<int> vec = {1, 2, 3, 4, 5, 6};

    ListNode* root = sol.reverseList(sol.createList(vec));
    ListNode* next;

    while(root)
    {
        cout << root->val << endl;
        next = root->next;
        delete(root);
        root = next;
    }

    return 0;
}
