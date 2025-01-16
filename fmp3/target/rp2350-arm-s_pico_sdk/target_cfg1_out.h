/*
 * cfg1_out.cをリンクするために必要なスタブの定義
 */

void _kernel_start(void){}

void _close_r(){}
void _lseek_r(){}
void _read_r(){}
void _write_r(){}

void main(){}

/*
 * チップ依存のスタブの定義 
 */
#include <chip_cfg1_out.h>
