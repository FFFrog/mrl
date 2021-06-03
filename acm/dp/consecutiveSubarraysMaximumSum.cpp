//**********************************************************************************************
//
//    Filename:         consecutiveSubarraysMaximumSum.cpp
//    Author:           lijiawei
//    Create at:        2021/06/03 15:29:37
//    Link:             https://leetcode-cn.com/problems/lian-xu-zi-shu-zu-de-zui-da-he-lcof/
//    Description:      Consecutive Subarray Maximum Sum
//
//**********************************************************************************************

#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    int maxSubArray(vector<int> &nums)
    {
        int n = nums.size();

        vector<int> dp;
        dp.resize(n);
        dp[0] = nums[0];
        int max = nums[0];

        for (int i = 1; i < n; i++) {
            dp[i] = dp[i - 1] > 0 ? dp[i - 1] + nums[i] : nums[i];
            max = dp[i] > max ? dp[i] : max;
        }

        return max;
    }
};

int main()
{
    vector<int> vec = {-2, 1, -3, 4, -1, 2, 1, -5, 4};

    Solution sol;
    cout << sol.maxSubArray(vec) << endl;

    return 0;
}
