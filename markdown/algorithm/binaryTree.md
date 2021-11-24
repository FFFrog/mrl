# 二叉树

## 构造

```c++
// 递归
TreeNode* buildTreebyRecursionOne(int index = 0)
{
    TreeNode *node = nullptr;

    if (index < vRoot.size()) {
        node = new TreeNode(stoi(vRoot[index]));

        node->left = buildTreebyRecursionOne(2 * index + 1);
        node->right = buildTreebyRecursionOne(2 * index + 2);
    }

    return node;
}

// 迭代
TreeNode* buildTreebyIteration()
{
    queue<TreeNode *> que;
    TreeNode* root = nullptr;
    int i = 1;

    if (!vRoot.empty()) {
        root = new TreeNode(stoi(vRoot[0]));
        que.push(tRoot);

        while (i < vRoot.size()) {
            TreeNode *root = que.front();
            root->left = new TreeNode(stoi(vRoot[i++]));
            root->right = new TreeNode(stoi(vRoot[i++]));
            que.pop();
            que.push(root->left);
            que.push(root->right);
        }
    }

    return root;
}
```

## 遍历

```c++
// 递归
void preTraversalbyRecursion(TreeNode *root, vector<int> &vec)
{
    if (root) {
        // 先序遍历
        preTraversalbyRecursion(root->left, vec);
        preTraversalbyRecursion(root->right, vec);
    }
}

void inTraversalbyRecursion(TreeNode *root, vector<int> &vec)
{
    if (root) {
        inTraversalbyRecursion(root->left, vec);
        // 中序遍历
        inTraversalbyRecursion(root->right, vec);
    }
}

void postTraversalbyRecursion(TreeNode *root, vector<int> &vec)
{
    if (root) {
        postTraversalbyRecursion(root->left, vec);
        postTraversalbyRecursion(root->right, vec);
        // 后序遍历
    }
}

// 迭代
void preOrderTraversalbyIteration(TreeNode* root)
{
    stack<TreeNode *> st;

    while (root != nullptr || !st.empty()) {
        while (root != nullptr) {
            // 先序遍历
            st.push(root);
            root = root->left;
        }

        TreeNode *node = st.top();
        st.pop();
        root = node->right;
    }
}

void inOrderTraversalbyIteration(TreeNode* root)
{
    stack<TreeNode *> st;

    while (root != nullptr || !st.empty()) {
        while (root != nullptr) {
            st.push(root);
            root = root->left;
        }

        TreeNode *node = st.top();
        st.pop();
        // 中序遍历
        root = node->right;
    }
}

void postOrderTraversalbyIteration(TreeNode* root)
{
    stack<TreeNode *> st;
    TreeNode *last = nullptr;

    while (root != nullptr || !st.empty()) {
        while (root != nullptr) {
            st.push(root);
            root = root->left;
        }

        TreeNode *node = st.top();
        if (node->right == nullptr || node->right == last) {
            last = node;
            st.pop();
            // 后序遍历
        } else
            root = node->right;
    }
}
```
