/*
 * シリアルインタフェースドライバのターゲット依存部（非TECS版専用）
 *
 * $Id: target_serial.h 289 2021-08-05 14:44:10Z ertl-komori $
 */

#include <stdint.h>
#include <stdio.h>
#include "t_stddef.h"
#include "target_serial.h"
#include "target_syssvc.h"

struct sio_port_control_block
{
    intptr_t exinf;
    FILE *file;
};

static SIOPCB siopcb[TNUM_PORT];

/*
 * SIOドライバの初期化
 */
void sio_initialize(intptr_t exinf)
{
}

/*
 * SIOドライバの終了処理
 */
void sio_terminate(intptr_t exinf)
{
}

/*
 * SIOポートのオープン
 */
SIOPCB *sio_opn_por(ID siopid, intptr_t exinf)
{
    SIOPCB *result = NULL;
    if (siopid >= TNUM_PORT)
    {
        return result;
    }
    result = &siopcb[siopid];
    result->exinf = exinf;
    switch (siopid)
    {
    case 1:
        result->file = stdout;
        break;
    case 2:
        result->file = stderr;
        break;
    default:
        return NULL;
    }
    return result;
}

/*
 * SIOポートのクローズ
 */
void sio_cls_por(SIOPCB *p_siopcb)
{
    p_siopcb->file = NULL;
}

/*
 * SIOポートへの文字送信
 */
bool_t sio_snd_chr(SIOPCB *p_siopcb, char c)
{
    return putc(c, p_siopcb->file) != EOF;
}

/*
 * SIOポートからの文字受信
 */
int_t sio_rcv_chr(SIOPCB *p_siopcb)
{
    return getc(stdin);
}

/*
 * SIOポートからのコールバックの許可
 */
void sio_ena_cbr(SIOPCB *p_siopcb, uint_t cbrtn)
{
    switch (cbrtn)
    {
    case SIO_RDY_SND:
        /* code */
        break;
    case SIO_RDY_RCV:
        /* code */
        break;
    default:
        break;
    }
}

/*
 * SIOポートからのコールバックの禁止
 */
void sio_dis_cbr(SIOPCB *p_siopcb, uint_t cbrtn)
{
    switch (cbrtn)
    {
    case SIO_RDY_SND:
        /* code */
        break;
    case SIO_RDY_RCV:
        /* code */
        break;
    default:
        break;
    }
}

/*
 * SIOポートへの文字出力
 */
void target_fput_log(char c)
{
    if (c == '\n')
    {
        putc('\r', stdout);
    }
    putc(c, stdout);
}
