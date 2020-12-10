using System;
using System.Collections.Generic;
using Microsoft.AspNetCore.Mvc.Rendering;

using static lab2.Controllers.HomeController.State;

namespace lab2.Models {
    public class ScheduleModel {
        public Schedule   Schedule { get; }
        public SelectList Rooms    { get; }
        public String[]   Days     { get; }
        public String[]   Times    { get; }

        public ScheduleModel(
            Schedule    schedule,
            SelectList  rooms,
            String[]    days,
            String[]    times
        ) {
            Schedule = schedule;
            Rooms    = rooms;
            Days     = days;
            Times    = times;
        }

        public Boolean IsRoomSelected => Schedule != null;
    }
}