#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    int numTrees(int n)
    {
        vector<int> ans(n + 1, 0);
        ans[0] = 1;

        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= i; j++) {
                ans[i] += ans[j - 1] * ans[i - j];
            }
        }

        return ans[n];
    }

    int numTrees2(int n)
    {
        int ans = 0;

        if (n == 0 || n == 1) {
            ans = 1;
        } else {
            for (int i = 1; i <= n; i++) {
                ans += numTrees2(i - 1) * numTrees2(n - i);
            }
        }

        return ans;
    }
};

int main()
{
    int n;

    cin >> n;

    Solution sol;
    cout << sol.numTrees(n) << endl;

    cout << sol.numTrees2(n) << endl;

    return 0;
}
