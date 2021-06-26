using System;
using System.Collections.Generic;

using static lab2.Controllers.HomeController.State;

namespace lab2.Models {
    public class ListModel {
        public String       Type  { get; set; }
        public List<String> List  { get; set; }
        public String       ToAdd { get; set; }
        public String       ToDel { get; set; }
    }
}