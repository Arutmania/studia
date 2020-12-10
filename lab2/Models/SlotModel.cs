using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.Rendering;
using Microsoft.Extensions.Logging;

//using System.Web.Mvc;

using static lab2.Controllers.HomeController.State;

namespace lab2.Models {
    public class SlotModel {
        public String Room { get; set; }

        public Int32  Day  { get; set; }
        public Int32  Time { get; set; }

        public SelectList Groups   { get; set; }
        public SelectList Lectures { get; set; }
        public SelectList Teachers { get; set; }

        public String SelectedGroup   { get; set; }
        public String SelectedLecture { get; set; }
        public String SelectedTeacher { get; set; }
    }
}