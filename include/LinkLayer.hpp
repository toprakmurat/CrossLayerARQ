#pragma once

#include "Engine.hpp"
#include "Packets.hpp"
#include <deque>
#include <memory>
#include <vector>

namespace ARQ {

class PhysicalLayer;
class TransportLayer;

class LinkLayer : public std::enable_shared_from_this<LinkLayer> {
public:
  LinkLayer(SimulatorEngine &engine, uint32_t window_size);

  void SetLowerLayer(std::shared_ptr<PhysicalLayer> phy);
  void SetUpperLayer(std::shared_ptr<TransportLayer> transport);

  // --- SENDER API (Downstream) ---
  void Send(std::pmr::vector<std::byte> transport_segment);

  // --- RECEIVER API (Upstream) ---
  void Receive(std::shared_ptr<const Frame> frame_ptr);

  // --- CONTROL API ---
  void SetPaused(bool paused);

private:
  SimulatorEngine &engine_;
  std::shared_ptr<PhysicalLayer> phy_;
  std::weak_ptr<TransportLayer> transport_;
  uint32_t window_size_;
  bool is_paused_ = false;
  CRC32 crc32_;

  uint32_t next_seq_num_ = 0;
  uint32_t send_base_ = 0;

  struct QueuedItem {
    std::pmr::vector<std::byte> payload;
  };
  std::deque<QueuedItem> pending_queue_;

  // --- Sender Window ---
  struct TxWindowSlot {
    std::shared_ptr<Frame> frame;
    bool acked = false;
    uint64_t timer_event_id = 0;
  };
  std::deque<TxWindowSlot> send_window_;

  // --- Receiver Window ---
  struct RxWindowSlot {
    std::shared_ptr<Frame> frame;
    bool received = false;
  };
  std::vector<RxWindowSlot> rx_window_;
  uint32_t rx_base_ = 0;

  void HandleAck(std::shared_ptr<const Frame> frame);
  void HandleData(std::shared_ptr<const Frame> frame);
  void SendAck(uint32_t ack_num);
  void TrySendNext();
  void HandleTimeout(uint32_t seq_num);
  void ComputeAndSetChecksum(std::shared_ptr<Frame> frame);
};

} // namespace ARQ
