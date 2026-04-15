import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15

ApplicationWindow {
    id: win
    width: 900
    height: 500
    visible: true
    title: "Othello QML App"

	// Called when the component is fully loaded
    Component.onCompleted: {
	}


	RowLayout {
		anchors.fill: parent
		spacing: 0
			        
	    Canvas {
	        id: canvas
        	width: 500;
	        height: 500;
	        Layout.preferredWidth: 500
        	Layout.preferredHeight: 500
	        
	        onPaint: {	        
	            var ctx = getContext("2d");
	            ctx.clearRect(0,0,canvas.width,canvas.height);
	            ctx.fillStyle = "#17a631"; // color or gradient
	            var h = 30;
	            var d = Math.min(canvas.width, canvas.height) - 2 * h;
	            //console.log("d=",d);
	            ctx.fillRect(h,h,d,d);
	            var scale = d / 8;
	            //draw grid
	            ctx.lineWidth = 2;
		        ctx.strokeStyle = "black";
	            ctx.beginPath();
	            var l = Math.min(canvas.width, canvas.height)
	            ctx.font = Math.round(d/20) +"px Serif";
        		ctx.fillStyle = "black";
	            for (var i = 0; i <= 8; ++i) {
	            	ctx.moveTo(h, i * scale + h);ctx.lineTo(l - h, i * scale + h);
	            	ctx.moveTo(i * scale + h, h);ctx.lineTo(i * scale + h, l - h);
	            }
	            ctx.stroke();
	            for (var i = 0; i < 8; ++i) {
	        		ctx.fillText(i + 1, h/4, h+h/4+d/16+i*d/8);
	        		ctx.fillText(String.fromCharCode(97 + i), h-h/4+d/16+i*d/8, d+d/16+0.75*h );
	            }
	            
	            var boardMap = gameModel.getBoard();
	            // map may already be a JS object; treat it as one
			    for (var key in boardMap) {
			        if (boardMap.hasOwnProperty(key)) {
			            var values = boardMap[key]			            
			            for(var v in values) {
				            var x = h + d / 8 * values[v].x	- d/ 16
				            var y = h + d / 8 * values[v].y	- d/ 16
				            ctx.beginPath()
				            ctx.arc(x, y, d / 18, 0, Math.PI * 2) // x, y, radius
				            ctx.fillStyle = key
				            ctx.fill()	            
			            }
			        }
			    }
	        }	        
	        
		    MouseArea {
			    anchors.fill: parent
			    onClicked: function(mouse) {
			    	var i = Math.floor(8*(mouse.x - 30) / 440 + 1);
			    	var j = Math.floor(8*(mouse.y - 30) / 440 + 1);
			    	var p = 10 * i + j;
			        console.log("clicked at: ", 
			        	Math.floor(8*(mouse.x - 30) / 440 + 1), 
			        	Math.floor(8*(mouse.y - 30) / 440 + 1));
			        var hasPlayed = gameModel.play(p);
			    }
			}
		}
		
		Connections {
	        target: gameModel
	        function onRequestPaint() {
	            // use requestPaint() for an immediate paint cycle
	            // or use requestAnimationFrame(function() { canvas.requestPaint(); });
	            canvas.requestPaint();
	        }
		}

		Item {
	        Layout.fillWidth: true
	        Layout.fillHeight: true
	
			ColumnLayout {
		        spacing: 12
		        Layout.fillWidth: true; 
		        Layout.fillHeight: true; 
				
				Text { 
					font.pointSize: 12
					text: gameModel.playerDesc + gameModel.playerMove 
				}
				Text { 
					font.pointSize: 12
					text: gameModel.computerDesc + gameModel.computerMove 
				}
		
		        Button {
		        	font.pointSize: 12		        
		            text: "Nouvelle partie"
		            onClicked: { 
		            	gameModel.start(); 
		            }
		        }
				Text { 
					font.pointSize: 12
					text: "" + gameModel.info 
				}
		    }
	    }
    }
}