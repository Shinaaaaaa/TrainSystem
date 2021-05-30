//
// Created by lenovo on 2021/5/22.
//
#include "Ticket.h"

Ticket::Ticket(const String<21> &trainID , int stationNo, const date &startDayTime , const date &startDay, const date &endDay , int arrivalTime , int leaveTime , int price) {
    TrainID = trainID;
    StationNo = stationNo;
    StartDayTime = startDayTime;
    StartDay = startDay;
    EndDay = endDay;
    Price = price;
    ArrivalTime = arrivalTime;
    LeaveTime = leaveTime;
}

Ticket &Ticket::operator=(const Ticket &t){
    if (this == &t) return *this;
    TrainID = t.TrainID;
    StationNo = t.StationNo;
    StartDayTime = t.StartDayTime;
    StartDay = t.StartDay;
    EndDay = t.EndDay;
    Price = t.Price;
    ArrivalTime = t.ArrivalTime;
    LeaveTime = t.LeaveTime;
    return *this;
}

Ticket::Ticket(const Ticket &t) {
    TrainID = t.TrainID;
    StationNo = t.StationNo;
    StartDayTime = t.StartDayTime;
    StartDay = t.StartDay;
    EndDay = t.EndDay;
    Price = t.Price;
    ArrivalTime = t.ArrivalTime;
    LeaveTime = t.LeaveTime;
}

bool Ticket::operator<(const Ticket &t) const {
    return TrainID < t.TrainID;
}
bool Ticket::operator>(const Ticket &t) const {
    return TrainID > t.TrainID;
}

bool Ticket::operator==(const Ticket &rhs) const {
    return TrainID == rhs.TrainID;
}

bool Ticket::operator!=(const Ticket &rhs) const {
    return !(rhs == *this);
}

bool Ticket::operator<=(const Ticket &rhs) const {
    return !(rhs < *this);
}

bool Ticket::operator>=(const Ticket &rhs) const {
    return !(*this < rhs);
}


void ticket_System::addTicket(const String<40> &station, const Ticket &t) {
    ticket_BPT.insert(station , t);
}

void ticket_System::delTicket(const String<40> &station, const Ticket &t) {
    ticket_BPT.erase(station , t);
}

void ticket_System::addTicket(const Train &t) {
    addTicket(t.Stations[1] ,Ticket (t.TrainID , 1 , t.StartDayTime ,  t.SaleDate_begin , t.SaleDate_end , 0 , 0 , 0));
    for (int i = 2 ; i <= t.StationNum ; ++i){
        Ticket tmp(t.TrainID , i ,  t.StartDayTime , t.SaleDate_begin , t.SaleDate_end , t.TravelTimeSum[i] + t.StopoverTimeSum[i - 1]
                   , t.TravelTimeSum[i] + t.StopoverTimeSum[i] , t.PriceSum[i]);
        addTicket(t.Stations[i] , tmp);
    }
}

void ticket_System::buyTicket(const String<21> &username , const Train &t, const String<40> &st, const String<40> &ed, const date &d,
                         int num, int isQue , int OrderNo) {//num为车票数
    if (t.IsRelease == 0) throw "error";
    int s = 0 , e = 0;
    for (int i = 1 ; i <= t.StationNum ; ++i){
        if (t.Stations[i] == st) s = i;
        else if (t.Stations[i] == ed) e = i;
    }
    if (s == 0 || e == 0) throw "error";
    date leave = t.SaleDate_begin; leave += t.StartDayTime;//leave为到st的时间
    date arrive = leave;
    arrive += date(0 , 0 , 0 , t.TravelTimeSum[e] + t.StopoverTimeSum[e - 1]);//arrive为到达ed的时间
    leave += date (0 , 0 , 0 , t.TravelTimeSum[s] + t.StopoverTimeSum[s]);
    int no = d - leave;
    if (no <= 0 || no > t.SaleDate_end - t.SaleDate_begin) throw "error";
    int l_ticketNum = trainControl.getSeatNum(t.TrainID , s , e , no);
    if (l_ticketNum >= num) {
        trainControl.modifySeat(t.TrainID , s , e , no , num);
        cout << (t.PriceSum[e] - t.PriceSum[s]) * num << "\n";
        Order o(OrderNo , 1 , t.TrainID , username , st , ed , leave + date(0, no - 1, 0, 0) , arrive + date(0, no - 1, 0, 0)
                , t.PriceSum[e] - t.PriceSum[s] , no , num);
        orderControl.addOrder(username , o);
    }
    else if (isQue == 0) {
        throw "error no seat";
    }
    else {
        Order o(OrderNo , 2 , t.TrainID , username , st , ed , leave + date(0, no - 1, 0, 0) , arrive + date(0, no - 1, 0, 0)
                , t.PriceSum[e] - t.PriceSum[s] , no , num);
        orderControl.addOrder(username , o);

        int pendingNum = trainControl.addPendingOrderNum(t , no);
        orderControl.addPendingOrder(o , no , pendingNum);
        cout << "queue\n";
    }
}

void ticket_System::que_BuyTicket(const String<21> &username , const Order &refund_o) {
    vector<pair<int , Order>> tmp = orderControl.findPendingOrder(make_pair(refund_o.TrainID , refund_o.No));
    tmp.sort();
    for (int i = 0 ; i < tmp.size() ; ++i){
        pair<int , Order> t = tmp[i];
        Order o = t.second;
        int left = trainControl.getSeatNum(o.TrainID , o.From , o.To , o.No);
        if (o.Num <= left) {
            trainControl.modifySeat(o.TrainID , o.From , o.To , o.No , o.Num);
            orderControl.delPendingOrder(o , o.No , t.first);
            Order tmpo = o;
            tmpo.changeStatus(1);
            orderControl.modifyOrder(username , o , tmpo);
        }
    }
}

vector<pair<Ticket, Ticket>> ticket_System::queryTicket(const String<40> &st , const String<40> &ed , const date &d) {
    vector<Ticket> begin_station = ticket_BPT.find(String<40> (st));
    vector<Ticket> end_station = ticket_BPT.find(String<40> (ed));
    begin_station.sort();
    end_station.sort();

    vector<pair<Ticket , Ticket>> ans;
    int p1 = 0 , p2 = 0;
    while (p1 < begin_station.size() && p2 < end_station.size()){
        if (begin_station[p1] == end_station[p2]) {
            if (begin_station[p1].StationNo > end_station[p2].StationNo) {
                p1++ ; p2++;
                continue;
            }
            date saleend = begin_station[p1].EndDay + begin_station[p1].StartDayTime + date(0,0,0,begin_station[p1].LeaveTime);
            if (saleend - d > 0){
                ans.push_back(make_pair(begin_station[p1] , end_station[p2]));
            }
            p1++ ; p2++;
        }
        else if (begin_station[p1] < end_station[p2]) p1++;
        else p2++;
    }
    return ans;
}

vector<pair<pair<int , Train> , pair<Ticket , Ticket>>> ticket_System::queryTransfer(const Train &t , const String<40> &st , const String<40> &ed , const date &d , int type) {
    vector<pair<int , int>> sort_contain;//排序用
    vector<pair<int , pair<Ticket , Ticket>>> sort_contain2;
    int start = 0;
    for (int i = 1 ; i <= t.StationNum ; ++i){//枚举中转站
        if (t.Stations[i] == st) {
            start = i;
            continue;
        }
        if (start == 0) continue;
        vector<pair<Ticket , Ticket>> tmp1;
        tmp1 = queryTicket(t.Stations[i] , ed , d);//查询中转站到ed的所有火车
        if (tmp1.empty()) continue;
        if (type == 0){//time
            for (int j = 0 ; j < tmp1.size() ; ++j){
                date arriveTrans = t.StartDayTime + t.SaleDate_begin + date (0,0,0,t.TravelTimeSum[i] + t.StopoverTimeSum[i-1]);
                int trainNo = d - arriveTrans;
                arriveTrans += date (0,trainNo - 1,0,0);

                Ticket trans = tmp1[j].first;
                Ticket end = tmp1[j].second;
                if (arriveTrans < trans.StartDay + trans.StartDayTime){
                    int wait = calMinute(trans.StartDay + trans.StartDayTime , arriveTrans);
                    sort_contain.push_back(make_pair(abs(t.TravelTimeSum[trans.StationNo] + t.StopoverTimeSum[trans.StationNo] - t.TravelTimeSum[start] - t.StopoverTimeSum[start] + wait + end.ArrivalTime - trans.LeaveTime) , j));
                }
                else {
                    if (t.StartDayTime < trans.StartDayTime){
                        int wait = calMinute(trans.StartDayTime , t.StartDayTime);
                        sort_contain.push_back(make_pair(abs(t.TravelTimeSum[trans.StationNo] + t.StopoverTimeSum[trans.StationNo] - t.TravelTimeSum[start] - t.StopoverTimeSum[start] + wait + end.ArrivalTime - trans.LeaveTime) , j));
                    }
                    else {
                        int wait = calMinute(trans.StartDayTime + date (0,1,0,0) , t.StartDayTime);
                        sort_contain.push_back(make_pair(abs(t.TravelTimeSum[trans.StationNo] + t.StopoverTimeSum[trans.StationNo] - t.TravelTimeSum[start] - t.StopoverTimeSum[start] + wait + end.ArrivalTime - trans.LeaveTime) , j));
                    }
                }
            }
            sort_contain.sort();
            sort_contain2.push_back(make_pair(sort_contain[0].first , tmp1[sort_contain[0].second]));
        }
        else {//cost
            Ticket trans = tmp1[i].first;
            Ticket end = tmp1[i].second;
            for (int j = 0 ; j < tmp1.size() ; ++j){
                sort_contain.push_back(make_pair(abs(t.PriceSum[i] - t.PriceSum[start] + end.Price - trans.Price) , j));
            }
            sort_contain.sort();
            sort_contain2.push_back(make_pair(sort_contain[0].first , tmp1[sort_contain[0].second]));
        }
    }
    sort_contain2.sort();
    vector<pair<pair<int , Train> , pair<Ticket , Ticket>>> ans_vec;
    if (!sort_contain2.empty()){
        ans_vec.push_back(make_pair(make_pair(sort_contain2[0].first , t) , sort_contain2[0].second));
    }
    return ans_vec;
}

void ticket_System::queryTransfer(const String<40> &st , const String<40> &ed , const date &d , int type) {
    vector<Ticket> begin_station = ticket_BPT.find(st);
    vector<pair<pair<int , Train> , pair<Ticket , Ticket>>> ans_con;
    for (int i = 0 ; i < begin_station.size() ; ++i){
        Ticket ticket = begin_station[i];
        date salebegin = begin_station[i].StartDay + begin_station[i].StartDayTime + date(0,0,0,begin_station[i].LeaveTime);
        date saleend = begin_station[i].EndDay + begin_station[i].StartDayTime + date(0,0,0,begin_station[i].LeaveTime);
        if (d - salebegin > 0 && saleend - d > 0){
            vector<Train> train_container = trainControl.find(ticket.TrainID);
            Train t = train_container[0];
            vector<pair<pair<int , Train> , pair<Ticket , Ticket>>> way = queryTransfer(t , String<40> (st) , String<40> (ed) , d , type);
            if (!way.empty()) ans_con.push_back(way[0]);
        }
    }
    if (ans_con.empty()) {
        cout << 0 << "\n";
        return;
    }
    ans_con.sort();
    pair<pair<int , Train> , pair<Ticket , Ticket>> answer = ans_con[0];
    Train firstTrain = answer.first.second;
    vector<Train> two = trainControl.find(answer.second.first.TrainID);
    Train secondTrain = two[0];
    int start , trans;
    String<40> transfer = secondTrain.Stations[answer.second.first.StationNo];
    for (int i = 0 ; i < firstTrain.StationNum ; ++i){
        if (firstTrain.Stations[i] == st) start = i;
        else if (firstTrain.Stations[i] == transfer) trans = i;
    }
    cout << firstTrain.TrainID << " " << st << " ";
    date tmp1 = firstTrain.StartDayTime; tmp1 += firstTrain.SaleDate_begin;
    date tmp2 = tmp1;//tmp2为到trans时间
    tmp2 += date(0 , 0 , 0 , firstTrain.TravelTimeSum[trans] + firstTrain.StopoverTimeSum[trans - 1]);
    tmp1 += date (0 , 0 , 0 , firstTrain.TravelTimeSum[start] + firstTrain.StopoverTimeSum[start]);
    int no = d - tmp1;
    tmp1 += date(0 , no - 1 , 0 , 0);
    tmp2 += date(0 , no - 1 , 0 , 0);
    tmp1.show();
    cout << " -> " << transfer << " ";
    tmp2.show();
    cout << " " << abs(firstTrain.PriceSum[trans] - firstTrain.PriceSum[start]) << " ";
    cout << trainControl.getSeatNum(firstTrain.TrainID , start , trans , no) << "\n";

    int no2;
    cout << secondTrain.TrainID << " " << transfer << " ";
    date tmp3 = secondTrain.StartDayTime; tmp3 += secondTrain.SaleDate_begin;
    date tmp4 = tmp3;
    tmp3 += date (0 , 0 , 0 ,answer.second.first.LeaveTime);
    tmp4 += date (0 , 0 , 0 ,answer.second.second.ArrivalTime);

    if (secondTrain.SaleDate_begin - tmp2 > 0){//在第二班车开始发车前到达trans
        no2 = 1;
    }
    else {
        no2 = tmp2 - secondTrain.StartDayTime;
        if (secondTrain.StartDayTime < firstTrain.StartDayTime) no2++;
    }
    tmp3 += date (0 , no2 - 1, 0 , 0);
    tmp4 += date (0 , no2 - 1, 0 , 0);
    tmp3.show();
    cout << " -> " << ed << " ";
    tmp4.show();
    cout << abs(answer.second.first.Price - answer.second.second.Price) << " ";
    cout << trainControl.getSeatNum(secondTrain.TrainID , transfer , ed , no2) << "\n";
}

vector<Ticket> ticket_System::find(const String<40> &trainID) {
    vector<Ticket> tmp = ticket_BPT.find(trainID);
    return tmp;
}
