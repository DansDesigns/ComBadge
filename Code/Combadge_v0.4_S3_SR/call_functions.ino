// ==================== Call State Handlers ====================
// These are called when a call ends to reset the relevant flag.
// "calling_badge" means we ARE calling a badge → reset call_badge flag on end.
// "calling_phone" means we ARE calling a phone → reset call_phone flag on end.

void calling_badge()
{
  call_badge = false;   // was incorrectly setting call_phone before
}

void calling_phone()
{
  call_phone = false;   // was incorrectly setting call_badge before
}

// ==================== Dial Digit Handlers ====================
// These build up a dialled number during an active call_badge or call_phone.
// Stubbed for now — will be wired into call_manager when contacts are added.

void number_0() { /* TODO: append '0' to dialled number */ }
void number_1() { /* TODO: append '1' to dialled number */ }
void number_2() { /* TODO: append '2' to dialled number */ }
void number_3() { /* TODO: append '3' to dialled number */ }
void number_4() { /* TODO: append '4' to dialled number */ }
void number_5() { /* TODO: append '5' to dialled number */ }
void number_6() { /* TODO: append '6' to dialled number */ }
void number_7() { /* TODO: append '7' to dialled number */ }
void number_8() { /* TODO: append '8' to dialled number */ }
void number_9() { /* TODO: append '9' to dialled number */ }


//EOF
