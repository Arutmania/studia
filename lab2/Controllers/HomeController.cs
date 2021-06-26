using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;

using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.Rendering;
using Microsoft.Extensions.Logging;

using System.Text.Json;

using lab2.Models;

namespace lab2.Controllers {
    public class HomeController : Controller {
        private readonly ILogger<HomeController> _logger;

        public static class State {
            public static List<String> Groups   { get; set; } = new List<String>();
            public static List<String> Lectures { get; set; } = new List<String>();
            public static List<String> Rooms    { get; set; } = new List<String>();
            public static List<String> Teachers { get; set; } = new List<String>();

            public static readonly String[] Days  = new [] {
                "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"
            };

            public static readonly String[] Times = new [] {
                "08:00-08:45", "08:55-09:40", "09:50-10:35",
                "10:55-11:40", "11:50-12:35", "12:45-13:30",
                "13:40-14:25", "14:35-15:20", "15:30-16:15" 
            };

            public static String? Room { get; set; } = null;

            public class SchedDict {
                private Dictionary<String, Schedule> schedule = new Dictionary<String, Schedule>();
                public Schedule this[String room] {
                    // get the c++ behavior of inserting default initialized value if key doesn't exist
                    get {
                        if (!schedule.ContainsKey(room)) {
                            schedule.Add(room, new Schedule()); // default is fucking retarded
                        }
                        return schedule[room];
                    }
                    set {
                        if (!schedule.ContainsKey(room)) {
                            schedule.Add(room, value);
                        } else {
                            schedule[room] = value;
                        }
                    }
                }
                public ICollection<String>   Keys   { get => schedule.Keys;   }
                public ICollection<Schedule> Values { get => schedule.Values; }

                public Boolean Remove(String key) => schedule.Remove(key);
            }

            public static SchedDict Schedules = new SchedDict();

            public class Schedule {
                public class Slot {
                    public String Group   { get; set; } = String.Empty;
                    public String Lecture { get; set; } = String.Empty;
                    public String Teacher { get; set; } = String.Empty;

                    public Boolean IsEmpty => 
                        String.IsNullOrEmpty(Group)   && 
                        String.IsNullOrEmpty(Lecture) && 
                        String.IsNullOrEmpty(Teacher)
                    ;

                    public void Clear() {
                        Group   = String.Empty;
                        Lecture = String.Empty;
                        Teacher = String.Empty;
                    }
                }

                public Schedule() {
                    for (var time = 0; time < Times.Length; time++)
                        for (var day = 0; day < Days.Length; day++)
                            schedule[time, day] = new Slot();
                }

                private Slot[,] schedule = new Slot[Times.Length, Days.Length];

                public Slot this[Int32 time, Int32 day] {
                    get => schedule[time, day];
                    set => schedule[time, day] = value;
                }

            }

            private static void RemoveInvalidBy(Predicate<Schedule.Slot> pred) {
                foreach (var schedule in Schedules.Values)
                    for (var time = 0; time < Times.Length; time++)
                        for (var day = 0; day < Days.Length; day++)
                            if (pred(schedule[time, day]))
                                schedule[time, day].Clear();
            }

            public static void RemoveInvalidByGroup() => 
                RemoveInvalidBy(slot => !Groups.Contains(slot.Group))
            ;

            public static void RemoveInvalidByLecture() =>
                RemoveInvalidBy(slot => !Lectures.Contains(slot.Lecture))
            ;

            public static void RemoveInvalidByTeacher() =>
                RemoveInvalidBy(slot => !Teachers.Contains(slot.Teacher))
            ;
        }

        public HomeController(ILogger<HomeController> logger) {
            _logger = logger;
        }

        public IActionResult Index() {
            // if room is not set and there are rooms set room to the first
            if (State.Room == null && State.Rooms.Any())
                State.Room = State.Rooms.First();

            return View(
                new ScheduleModel(
                    State.Room != null ? State.Schedules[State.Room] : null,
                    new SelectList(State.Rooms, State.Room),
                    State.Days,
                    State.Times
                )
            );
        }

        public IActionResult AddGroup(String elem) {
            State.Groups.Add(elem);
            State.Groups = State.Groups
                .Where(s => !String.IsNullOrEmpty(s))
                .Distinct()
                .ToList();
            return RedirectToAction("ViewList", new { type = "Groups" });
        }

        public IActionResult AddLecture(String elem) {
            State.Lectures.Add(elem);
            State.Lectures = State.Lectures
                .Where(s => !String.IsNullOrEmpty(s))
                .Distinct()
                .ToList();
            return RedirectToAction("ViewList", new { type = "Lectures" });
        }

        public IActionResult AddRoom(String elem) {
            State.Rooms.Add(elem);
            State.Rooms = State.Rooms
                .Where(s => !String.IsNullOrEmpty(s))
                .Distinct()
                .ToList();
            return RedirectToAction("ViewList", new { type = "Rooms" });
        }

        public IActionResult AddTeacher(String elem) {
            State.Teachers.Add(elem);
            State.Teachers = State.Teachers
                .Where(s => !String.IsNullOrEmpty(s))
                .Distinct()
                .ToList();
            return RedirectToAction("ViewList", new { type = "Teachers" });
        }
        public IActionResult ViewList(String type) {
            switch (type) {
            default:
                return Index();
            case "Groups":
                return View(new ListModel { 
                    Type = type,
                    List = State.Groups 
                });
            case "Lectures":
                return View(new ListModel { 
                    Type = type,
                    List = State.Lectures 
                });
            case "Rooms":
                return View(new ListModel { 
                    Type = type,
                    List = State.Rooms 
                });
            case "Teachers":
                return View(new ListModel { 
                    Type = type,
                    List = State.Teachers 
                });
            }
        }

        public IActionResult DeleteListElem(String type, String elem) {
            switch (type) {
            default:
                return RedirectToAction("Index", "Home");
            case "Groups":
                State.Groups.Remove(elem);
                State.RemoveInvalidByGroup();
                return RedirectToAction("ViewList", new { type });

            case "Lectures":
                State.Lectures.Remove(elem);
                State.RemoveInvalidByLecture();
                return RedirectToAction("ViewList", new { type });

            case "Rooms":
                State.Rooms.Remove(elem);
                State.Schedules.Remove(elem);
                if (State.Room == elem)
                    State.Room = null;
                return RedirectToAction("ViewList", new { type });

            case "Teachers":
                State.Teachers.Remove(elem);
                State.RemoveInvalidByTeacher();
                return RedirectToAction("ViewList", new { type });
            }
        }

        public IActionResult SetRoom(String room) {
            if (State.Rooms.Contains(room))
                State.Room = room;
            return RedirectToAction("Index");
        }

        public IActionResult Slot(String room, Int32 time, Int32 day) {
            if (!State.Rooms.Contains(room) && 
                0 > time && time <= State.Times.Length &&
                0 > day  && day  <= State.Times.Length) {
                return RedirectToAction("Index");
            }

            var slot = State.Schedules[room][time, day];
            return View(new SlotModel {
                Room     = room,
                Time     = time,
                Day      = day,
                Groups   = new SelectList(State.Groups,   slot.Group),
                Lectures = new SelectList(State.Lectures, slot.Lecture),
                Teachers = new SelectList(State.Teachers, slot.Teacher)
            });
        }

        public IActionResult ClearSlot(String room, Int32 time, Int32 day) {
            if (State.Rooms.Contains(room) &&
                time >= 0 && time < State.Times.Length &&
                day  >= 0 && day  < State.Days.Length) {
                State.Schedules[room][time, day].Clear();
            }
            return RedirectToAction("Index");
        }

        public IActionResult SetSlot(SlotModel model) {
            if (State.Rooms.Contains(model.Room) &&
                model.Time >= 0 && model.Time < State.Times.Length &&
                model.Day  >= 0 && model.Day  < State.Days.Length  &&
                State.Groups.Contains(model.SelectedGroup) &&
                State.Lectures.Contains(model.SelectedLecture) &&
                State.Teachers.Contains(model.SelectedTeacher)) {
                State.Schedules[model.Room][model.Time, model.Day] = new State.Schedule.Slot {
                    Group   = model.SelectedGroup,
                    Lecture = model.SelectedLecture,
                    Teacher = model.SelectedTeacher
                };
            }
            return RedirectToAction("Index");
        }

        [ResponseCache(Duration = 0,
                       Location = ResponseCacheLocation.None,
                       NoStore  = true)]
        public IActionResult Error() => View(
            new ErrorViewModel { 
                RequestId = Activity.Current?.Id ?? HttpContext.TraceIdentifier
            }
        );
    }
}
