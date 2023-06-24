#ifndef BLINK_SIGNAL_H_
#define BLINK_SIGNAL_H_
#include "blink/machine.h"
#include "blink/windows.h"

#ifndef WINBLINK
bool IsSignalSerious(int);
bool IsSignalQueueable(int);
void SigRestore(struct Machine *);
bool IsSignalIgnoredByDefault(int);
#endif
#ifdef WINBLINK
void OnSignal(int);
#else
void OnSignal(int, siginfo_t *, void *);
#endif
void EnqueueSignal(struct Machine *, int);
#ifndef WINBLINK
void DeliverSignal(struct Machine *, int, int);
void TerminateSignal(struct Machine *, int, int);
int ConsumeSignal(struct Machine *, int *, bool *);
void DeliverSignalToUser(struct Machine *, int, int);
#endif

#endif /* BLINK_SIGNAL_H_ */
