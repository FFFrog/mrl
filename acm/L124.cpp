#include <iostream>
#include <cmath>

using namespace std;

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr)
    {
    }
    TreeNode(int x) : val(x), left(nullptr), right(nullptr)
    {
    }
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right)
    {
    }
};

class Solution {
public:
    int maxPathSum(TreeNode *root)
    {
        maxNode(root);
        return ans;
    }

    int maxNode(TreeNode *root)
    {
        if (root == nullptr)
            return 0;

        int lvalue, rvalue;

        lvalue = max(0, maxNode(root->left));
        rvalue = max(0, maxNode(root->right));

        ans = max(ans, lvalue + rvalue + root->val);

        return root->val + max(lvalue, rvalue);
    }

private:
    int ans = INT_MIN;
};
