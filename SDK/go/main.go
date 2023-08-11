package main

import (
	"fmt"
	"log"
	"sync"
	"time"

	"github.com/pebbe/zmq4"
	"github.com/vmihailenco/msgpack/v5"
)

type Quik struct {
	url     string
	zmq_ctx *zmq4.Context
	zmq_skt *zmq4.Socket
	zmq_pll *zmq4.Poller
}

func NewQuik(url string) (quik *Quik, err error) {
	quik = &Quik{}
	quik.url = url
	quik.zmq_ctx, err = zmq4.NewContext()
	if err != nil {
		return nil, err
	}

	quik.zmq_skt, err = quik.zmq_ctx.NewSocket(zmq4.REQ)
	if err != nil {
		return nil, err
	}
	quik.zmq_skt.SetLinger(0)

	quik.zmq_pll = zmq4.NewPoller()
	quik.zmq_pll.Add(quik.zmq_skt, zmq4.POLLIN)

	return quik, nil
}

func (q *Quik) Connect() error {
	return q.zmq_skt.Connect(q.url)
}

func (q *Quik) Close() error {
	return q.zmq_skt.Close()
}

func (q *Quik) Invoke(args ...interface{}) {
	b, err := msgpack.Marshal(args)
	if err != nil {
		log.Println(err)
	}
	q.zmq_skt.SendBytes(b, zmq4.DONTWAIT)

	sockets, _ := q.zmq_pll.Poll(-1)
	for _, socket := range sockets {
		switch s := socket.Socket; s {
		case q.zmq_skt:
			s.Recv(zmq4.SNDMORE)
			s.RecvBytes(zmq4.Flag(0))
		}
	}

}

func (q *Quik) IsConnected() bool {
	return rpc_call[bool](q, "isConnected")
}

func (q *Quik) GetScriptPath() string {
	return rpc_call[string](q, "getScriptPath")
}

func (q *Quik) GetSecurityInfo(board string, ticker string) map[string]interface{} {
	return rpc_call[map[string]interface{}](q, "getSecurityInfo", board, ticker)
}

func rpc_call[T string | interface{} | bool | int | map[string]interface{}](q *Quik, args ...interface{}) T {
	b, err := msgpack.Marshal(args)
	if err != nil {
		log.Println(err)
	}
	q.zmq_skt.SendBytes(b, zmq4.DONTWAIT)

	sockets, _ := q.zmq_pll.Poll(-1)
	var result T

	for _, socket := range sockets {
		switch s := socket.Socket; s {
		case q.zmq_skt:
			s.Recv(zmq4.SNDMORE)
			bytes, _ := s.RecvBytes(zmq4.Flag(0))

			msgpack.Unmarshal(bytes, &result)
			return result
		}
	}
	return result
}

func main() {
	var wg sync.WaitGroup

	quik, _ := NewQuik("tcp://10.211.55.3:8004")
	quik.Connect()

	fmt.Println(quik.IsConnected())
	fmt.Println(quik.GetScriptPath())

	r := quik.GetSecurityInfo("TQBR", "SBER")
	for i := range r {
		fmt.Println(i, r[i])
	}

	t0 := time.Now()
	c0 := 0
	f0 := 100000
	for x := 0; x < 7; x++ {
		wg.Add(1)
		c0 += f0
		go func(id int) {
			quik, _ := NewQuik("tcp://10.211.55.3:8004")

			defer wg.Done()
			defer quik.Close()

			quik.Connect()

			for i := 0; i < f0; i++ {
				rpc_call[map[string]interface{}](quik, "getSecurityInfo", "SPBFUT", "SBER")
			}
		}(x)
	}
	wg.Wait()

	fmt.Println(float64(c0) / time.Since(t0).Seconds())
}
