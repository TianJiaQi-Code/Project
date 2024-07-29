#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

#define HOST "127.0.0.1"
#define PORT 3306
#define USER "root"
#define PASS "Abc13546137306"
#define DBNAME "gobang"

int main()
{
    //  1. 初始化mysql句柄
    MYSQL *mysql = mysql_init(NULL);
    if (mysql == NULL)
    {
        printf("mysql init failed!\n");
        return -1;
    }
    //  2. 连接服务器
    if (mysql_real_connect(mysql, HOST, USER, PASS, DBNAME, PORT, NULL, 0) == NULL)
    {
        printf("connect mysql server failed: %s\n", mysql_error(mysql));
        mysql_close(mysql);
        return -1;
    }
    //  3. 设置客户端字符集
    if (mysql_set_character_set(mysql, "utf8") != 0)
    {
        printf("set client character failed: %s\n", mysql_error(mysql));
        mysql_close(mysql);
        return -1;
    }
    //  4. 选择要操作的数据库
    // mysql_select_db(mysql, DBNAME);
    //  5. 执行sql语句
    // char *sql = "insert stu values(null, '小田', 20, 53, 68, 87);";
    // char *sql = "update stu set ch=ch+40 where sn=1;";
    // char *sql = "delete from stu where sn=1;";
    char *sql = "select * from stu";
    int ret = mysql_query(mysql, sql);
    if (ret != 0)
    {
        printf("%s\n", sql);
        printf("mysql query failed: %s\n", mysql_error(mysql));
        mysql_close(mysql);
        return -1;
    }
    //  6. 如果sql语句是查询语句, 则需要保存结果到本地
    MYSQL_RES *res = mysql_store_result(mysql);
    if (res == NULL)
    {
        mysql_close(mysql);
        return -1;
    }
    //  7. 获取结果集中的结果条数
    int num_row = mysql_num_rows(res);
    int num_col = mysql_num_fields(res);
    //  8. 遍历保存到本地的结果集
    for (int i = 0; i < num_row; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        for (int j = 0; j < num_col; j++)
        {
            printf("%s\t", row[j]);
        }
        printf("\n");
    }
    //  9. 释放结果集
    mysql_free_result(res);
    // 10. 关闭连接, 释放句柄
    mysql_close(mysql);
    return 0;
}