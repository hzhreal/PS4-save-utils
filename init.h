#ifndef INIT_H
#define INIT_H

#include <unistd.h>
#include <sys/stat.h>
#include <orbis/libkernel.h>
#include "ps4-libjbc/jailbreak.h"
#include "ps4-libjbc/utils.h"
#include "sd.h"
#include "scall.h"

int init_cred();
int init_devices();

#endif // INIT_H