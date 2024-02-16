#include <stdio.h>
// int main()
// {

//     printf("\033[31mRed\033[0m ");
//     printf("\033[32mGreen\033[0m ");
//     printf("\033[33mYellow\033[0m ");
//     printf("\033[34mBlue\033[0m ");
//     printf("\033[35mMagenta\033[0m ");
//     printf("\033[36mCyan\033[0m ");
//     printf("\033[37mWhite\033[0m\n");

//     printf("\033[41mRed background\033[0m\n");
//     printf("\033[42mGreen background\033[0m\n");
//     printf("\033[43mYellow background\033[0m\n");
//     printf("\033[44mBlue background\033[0m\n");
//     printf("\033[45mMagenta background\033[0m\n");
//     printf("\033[46mCyan background\033[0m\n");
//     printf("\033[47mWhite background\033[0m\n");
    
// }


char* custom_strrchr(char* str, char c) {
    char* found = NULL;
    while (*str) {
        if (*str == c) {
            found = str;
        }
        str++;
    }
    if (found != NULL) {
        *found = '\0';  // 将最后一个匹配字符位置的值设置为字符串结束符
    }
    return found;
}

int main() {
    char input[] = "example/string/";
    char* result = custom_strrchr(input, '/');
    if (result != NULL) {
        printf("找到斜杠，位置在 %ld\n", result - input);
        printf("截断后的字符串为：%s\n", input);  // 注意这里打印的是输入字符串的地址
    } else {
        printf("未找到斜杠\n");
    }
    return 0;
}