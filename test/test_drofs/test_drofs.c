#include <unity.h>


void setUp(){}
void tearDown(){}

void nothing(){

}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(nothing);
    return UNITY_END();
}

void app_main() {
    main();
}
