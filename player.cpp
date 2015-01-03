#include "player.h"
#include "parser.h"

#include <algorithm>
#include <functional>

void Player::run()
{
    std::list<MidiMessage *>::iterator it = messages.begin();

    auto startTime = std::chrono::high_resolution_clock::now();

    while (playing) {
        if (it == messages.end()) {
            break;
        }

        auto duration = std::chrono::high_resolution_clock::now() - startTime;
        auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        uint32_t current = msec.count();

        MidiMessage *message = *it;
        if (message->msec <= current) {
            client.send(message->bytes, message->length);
            ++it;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void Player::sendAllNoteOff()
{
    uint8_t bytes[3] = {0, 0x7B, 0x00};
    for(int i = 0; i < 16; ++i) {
        bytes[0] = 0xB0 | (0x0F & i);
        client.send(bytes, sizeof(bytes));
    }
}

void Player::stop()
{
    playing = false;
    sendAllNoteOff();
}

uint32_t Player::tick2msec(uint32_t tick)
{
    if (tempoTable.empty()) {
        return (uint32_t)(60000.0 / 120.0f / resolution * (tick - 0) + 0);
    }
    else {
        TempoInfo &target = tempoTable.front();
        for (TempoInfo &info : tempoTable) {
            if (tick < info.tick) {
                target = info;
            }
            else {
                break;
            }
        }

        return (uint32_t)(60000.0 / target.tempo / resolution * (tick - target.tick) + target.time);
    }
}

void Player::visit(ParseContext *context)
{
    if (playing) {
        playing = false;
        sendAllNoteOff();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    resolution = context->sequence->resolution;
    tempoTable.clear();

    for_each(messages.begin(), messages.end(), [](MidiMessage *message) { delete message; });
    messages.clear();

    for (Track *track : context->sequence->tracks) {
        for (MidiEvent *event : track->events) {
            event->accept(this);
        }
    }

    messages.sort([](MidiMessage *a, MidiMessage *b) { return a->tick < b->tick; });
    for (MidiMessage *message : messages) {
        message->msec = tick2msec(message->tick);
    }

    if (thread) {
        thread->join();
        delete thread;
    }

    thread = new std::thread(&Player::run, this);
    playing = true;
}

void Player::visit(TempoEvent *elem)
{
    tempoTable.push_back(TempoInfo(elem->tick, elem->tempo, tick2msec(elem->tick)));
}

void Player::visit(BankSelectEvent *elem)
{
    MidiMessage *message;

    message = (MidiMessage *)malloc(sizeof(MidiMessage));
    message->bytes[0] = 0xB0 | (0x0F & elem->channel);
    message->bytes[1] = 0x00;
    message->bytes[2] = elem->msb;
    message->length = 3;
    message->tick = elem->tick;
    messages.push_back(message);

    message = (MidiMessage *)malloc(sizeof(MidiMessage));
    message->bytes[0] = 0xB0 | (0x0F & elem->channel);
    message->bytes[1] = 0x20;
    message->bytes[2] = elem->lsb;
    message->length = 3;
    message->tick = elem->tick;
    messages.push_back(message);
}

void Player::visit(ProgramChangeEvent *elem)
{
    MidiMessage *message = (MidiMessage *)malloc(sizeof(MidiMessage));
    message->bytes[0] = 0xC0 | (0x0F & elem->channel);
    message->bytes[1] = elem->programNo;
    message->length = 2;
    message->tick = elem->tick;
    messages.push_back(message);
}

void Player::visit(NoteEvent *elem)
{
    MidiMessage *message;

    message = (MidiMessage *)malloc(sizeof(MidiMessage));
    message->bytes[0] = 0x90 | (0x0F & elem->channel);
    message->bytes[1] = elem->noteNo;
    message->bytes[2] = elem->velocity;
    message->length = 3;
    message->tick = elem->tick;
    messages.push_back(message);

    message = (MidiMessage *)malloc(sizeof(MidiMessage));
    message->bytes[0] = 0x80 | (0x0F & elem->channel);
    message->bytes[1] = elem->noteNo;
    message->bytes[2] = 0x00;
    message->length = 3;
    message->tick = elem->tick + elem->gatetime;
    messages.push_back(message);
}

void Player::visit(Sequence *elem){}
void Player::visit(Track *elem){}
void Player::visit(NameEvent *elem){}
void Player::visit(TimeSignatureEvent *elem){}
void Player::visit(MarkerEvent *elem){}
void Player::visit(TrackEndEvent *elem){}
