from datetime import datetime
from typing import Any
from uuid import uuid4

from pydantic import BaseModel, Field


class NormalizedFlowIn(BaseModel):
    event_id: str = Field(default_factory=lambda: str(uuid4()))

    source_type: str = "rest"
    sensor_id: str = "unknown"

    src_ip: str
    dst_ip: str
    src_port: int = 0
    dst_port: int = 0
    protocol: str

    bytes: int = 0
    packets: int = 0

    start_time: datetime
    end_time: datetime
    duration: float = 0.0

    app_protocol: str = ""
    tcp_flags: str = ""

    raw: dict[str, Any] = {}


class IngestResponse(BaseModel):
    status: str
    count: int
    topic: str
